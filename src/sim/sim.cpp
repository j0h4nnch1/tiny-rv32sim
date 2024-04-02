#include "sim/sim.h"
#include "memory/mem.h"
#include <cstring>
#include <sys/mman.h> //mmap
#include <fcntl.h>    //file
#include <unistd.h>
#include "common/elf.h"
#include <readline/readline.h> //for input

#include <fstream>
#include <vector>

void sim_t::load_payload(const std::string& payload, mmu_t& iv_mem) {
    std::ifstream file(payload, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        printf("Error: Could not open file.\n");
        return;
    }
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> ram_image(size);
    if (!file.read(reinterpret_cast<char*>(ram_image.data()), size)) {
        printf("Error: Could not open file.\n");
        return;
    }
    file.close();
    uint64_t reset_vector = RESET_VECTOR;
    memcpy(reinterpret_cast<void*>(iv_mem.guest2host(reset_vector)), ram_image.data(), size);
}

void sim_t::update_dtb(mmu_t& iv_mem, riscv32_cpu_state& cpu){
    uint32_t size_dtb = sizeof(default_dtb);
    uint32_t pos_dtb = MEM_RANGE - size_dtb - 192;
    memcpy(reinterpret_cast<void*>(iv_mem.guest2host(pos_dtb)), default_dtb, size_dtb);
    cpu.gpr[10].val = 0;
    cpu.gpr[11].val = MEM_RANGE - sizeof(default_dtb)-192;
    uint32_t dtb = MEM_RANGE - sizeof(default_dtb) - 192;
    
    if(iv_mem.read(dtb+0x13c, 4) == 0x00c0ff03){
        uint32_t validram = cpu.gpr[11].val - VADDR_BASE;
        uint32_t update = (validram>>24) | ((( validram >> 16 ) & 0xff) << 8 ) | (((validram>>8) & 0xff ) << 16 ) | ( ( validram & 0xff) << 24 );
        iv_mem.write(dtb + 0x13c, update);
    }
}

char* sim_t::get_filename(){
    return this->file;
}

void sim_t::reset(){
    printf("reset\n");
}

void sim_t::win_run(cpu_t& cpu, mmu_t& iv_mem, riscv32_cpu_state& state, uart& iv_uart, decode& iv_decode){
    cpu.init(state);
    char* str = nullptr;
    for( ; str = readline("(sim) > "); ){
        if(str==nullptr) continue;
        switch (*str){
            case 'r':
                cpu.fetch_decode_exec(iv_mem, state, iv_uart, iv_decode);
                break;
            case 'd':
                printf("dump reg\n");
                break;
            case 'h':
                printf("input r/d/h \n");
                break;
            case 'q':
                printf("exit emu\n");
                return ;
                break;
            default:
                printf("not support\n");
                break;
        }
    }
}

void sim_t::run(cpu_t& cpu, mmu_t& iv_mem, uart& iv_uart, decode& iv_decode, riscv32_cpu_state& state){
    iv_mem.init();// init MMU, nothing for now
    this->load_elf(iv_mem, this->file, state);
    this->win_run(cpu, iv_mem, state, iv_uart, iv_decode);
}

void sim_t::load_img(mmu_t& iv_mem){
    memcpy((void*)iv_mem.guest2host(RESET_VECTOR), (void*)(this->img), sizeof(this->img));
    uint32_t temp = iv_mem.read_host((void*)iv_mem.guest2host(RESET_VECTOR ), 4);
    printf("data in mem:%lx\n", temp);
}

std::map<std::string, uint32_t> sim_t::load_elf(mmu_t& iv_mem, const char* file, riscv32_cpu_state& state){
    uint32_t len = std::strlen(file);
    //todo:construct symbol
    std::map<std::string, uint32_t> symbols;
    if(file == nullptr){
        printf("load default img...\n");
        load_img(iv_mem);// using default
        std::map<std::string, uint32_t> symbols;
        return symbols;
    }
    const char* suffix = file + len - 4;
    if((len >= 4&&(std::strcmp(suffix, ".bin") == 0))||(std::strrchr(file, '.') == nullptr)){
        //.bin or no suffix
        load_payload(file, iv_mem);
        if(std::strrchr(file, '.') == nullptr){
            //update_dtb
            printf("load kernel\n");
            update_dtb(iv_mem, state);
        }
    }
    else{
        state.gpr[10].val = 0;
        state.gpr[11].val = MEM_RANGE - sizeof(default_dtb)-192;
        int fd = open(file, O_RDONLY);
        assert(fd >= 0);
        off_t size = lseek(fd, 0, SEEK_END);
        const char* buffer = (const char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        assert(buffer != MAP_FAILED);
        close(fd);
        //check elf file only elf32 now
        const header32* elf_header = (const header32*)buffer;
        bool iself = (elf_header->e_ident[0]==0x7f)&&(elf_header->e_ident[1]=='E') \
                    &&(elf_header->e_ident[2]=='L')&&(elf_header->e_ident[3]=='F');
        bool iself32 = elf_header->e_ident[4] == 1;
        assert(iself && iself32);
        //get programm section
        program_hdr32* phdr = (program_hdr32*)(buffer + elf_header->e_phoff);
        entry_point = elf_header->e_entry;
        //copy program to mem
        for(int i = 0; i < elf_header->e_phnum; i++){
            if((phdr[i].p_type == 1) && (phdr[i].p_memsz != 0) \
                && phdr[i].p_filesz!=0){
                memcpy((void*)iv_mem.guest2host(phdr[i].p_paddr), (void*)((uint8_t*)buffer + phdr[i].p_offset), phdr[i].p_filesz);
                // iv_mem.dump_memory(RESET_VECTOR, 14);
            }
        }
        munmap((void*)buffer, size);
    }

    return symbols;
}