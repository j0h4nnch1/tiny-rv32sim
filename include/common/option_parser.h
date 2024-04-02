#ifndef _OPTION_PARSER_H
#define _OPTION_PARSER_H

#include <functional>
#include <vector>
#include <sim/sim.h>

#ifdef __GNUC__
# define likely(x) __builtin_expect(x, 1)
# define unlikely(x) __builtin_expect(x, 0)
# define NOINLINE __attribute__ ((noinline))
# define NORETURN __attribute__ ((noreturn))
# define ALWAYS_INLINE __attribute__ ((always_inline))
# define UNUSED __attribute__ ((unused))
#else
# define   likely(x) (x)
# define unlikely(x) (x)
# define NOINLINE
# define NORETURN
# define ALWAYS_INLINE
# define UNUSED
#endif

class option_parser_t
{
private:
    struct option_t
    {
        char c;
        char* s;
        std::function<void(const char*)> action;
        option_t(char c, char* s, std::function<void(const char*)> action):
        c(c),s(s),action(action){}
    };
    std::vector<option_t> opts;
public:
    option_parser_t(){}
    void add_option(char c, char* s, std::function<void(const char*)> action);
    void parse(char** argv0);
    void help();
    void update_img(sim_t& sim, const char* img);
};
#endif
