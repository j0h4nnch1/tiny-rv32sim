#include "common/option_parser.h"
#include <cstring>
#include <cstdlib>

void option_parser_t::add_option(char c, char* s, std::function<void(const char*)> action){
    opts.push_back(option_t(c, s, action));    
}

void option_parser_t::parse(char** argv){
    char** argv1 = argv + 1;
    for(char* opt; ((opt=*argv1)!=NULL)&&opt[0] == '-'; argv1++){
        bool found = false;
        for(auto it = opts.begin(); !found && it != opts.end(); it++){
            size_t str_len = it->s == NULL? 0 : strlen(it->s);
            bool chr_match = (opt[1]!='-')&&(it->c!=NULL)&&(it->c==opt[1]);
            bool str_match = (opt[1]=='-')&&(str_len!=0)&&strncmp(opt+2, it->s, str_len);
            char* optarg = NULL;
            if(chr_match||str_match){
                optarg = opt + 3;
                found = true;
                it->action(optarg);
            }
        }
        if(!found){
            printf("parm not match\n");
        }
    }
}

void option_parser_t::help(){
    printf("help message\n");
}

void option_parser_t::update_img(sim_t& sim, const char* file){
    sim.file = (char *)file;
}