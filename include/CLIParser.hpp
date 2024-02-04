#ifndef COMMANDLINEPARSER_CLIPARSER_HPP
#define COMMANDLINEPARSER_CLIPARSER_HPP

#include <cstdarg>
#include <vector>
#include <exception>
#include <iostream>
#include <concepts>
#include <cstring>
#include <filesystem>
#include <type_traits>
#include <optional>

namespace cli {

#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

#ifndef CLI_PREFIX
#define CLI_PREFIX "--", "-"
#endif

#define NUMBER_OF_PREFIXES PP_NARG(CLI_PREFIX)
const char* prefixes[NUMBER_OF_PREFIXES] = {CLI_PREFIX};

    template <typename Type>
    concept OptType = requires(Type t) {
        {true};
    };

#define CLI_OPTION_REQUIRED 0b00000001
#define CLI_OPTION_PROMPT 0b00000011
#define CLI_OPTION_NULL 0

    template <typename UnderlyingType, uint8_t OPTION>
    class Opt {
    public:
        template<typename... Ts>
        Opt( const char* const name, const char* const description, Ts&... codes) : name(new char[strlen(name)]), description(new char[strlen(description)]) {
            strcpy(this->name, name);
            strcpy(this->description, description);
            for (const auto& pref : {codes...}) {
                numberOfCodes++;
            }
            if (numberOfCodes <= 0) throw std::invalid_argument("did not receive any opts");
            code = new char*[numberOfCodes];
            size_t i = 0;
            ([&]
            {
                const char* const op = codes;
                const size_t opsize = strlen(op) + 1;
                code[i] = new char[opsize];
                memcpy(code[i], op, opsize);
                i++;
            } (), ...);
            std::qsort( // make sure prefixes are in order of size
                    code,
                    numberOfCodes,
                    sizeof(typeof(code[0])),
                    [] (const void* x, const void* y) {
                        const char* const xPrefix = *static_cast<const char* const*>(x);
                        const char* const yPrefix = *static_cast<const char* const*>(y);
                        const auto cmp = strlen(yPrefix) <=> strlen(xPrefix);
                        if (cmp < nullptr) {
                            return -1;
                        } else if (cmp > nullptr) {
                            return 1;
                        }
                        return 0;
                    }
            );
            initializeVars();
        }
        ~Opt() {
            delete[] name;
            delete[] description;
            for (size_t i =0; i< numberOfCodes; i++) {
                delete[] code[i];
            }
            delete[] code;
        }

        [[nodiscard]] bool hasValue() const {
            return isSet;
        }

        void set(UnderlyingType input) {
            isSet = true;
            t = input;
        }

        operator UnderlyingType() const {
            if (!isSet) throw std::bad_optional_access();
            return t;
        }
    private:
//        template<OptType... Opts>
//        friend void parse(int argc, const char* const* argv, std::vector<const char*>* defaults, Opts&... options);
        template<OptType... Opts>
        friend void parse(int argc, const char* const* argv, Opts&... options);

        bool accepts(const char* const slice, size_t* sizeOfPrefix) {
            for (size_t i =0; i< numberOfCodes; i++) {
                if (strncmp(code[i], slice, strlen(code[i])) == 0) {
                    *sizeOfPrefix = strlen(code[i]);
                    return true;
                }
            }
            return false;
        }
        bool accepts(const char* const slice) {
            for (size_t i =0; i< numberOfCodes; i++) {
                if (strncmp(code[i], slice, strlen(code[i])) == 0) {
                    return true;
                }
            }
            return false;
        }

        template <uint8_t U = OPTION>
        requires (OPTION == CLI_OPTION_NULL)
        void cleanup(std::vector<std::exception*>* exceptions) {
        }

        template <uint8_t U = OPTION>
        requires ((OPTION & 3) == CLI_OPTION_PROMPT)
        void cleanup(std::vector<std::exception*>* exceptions) {
            std::cout << "Enter a value for " << name << " (" << description << ") :\n";
            std::string input;
            std::cin >> input;
            parse(input);
        }

        template <uint8_t U = OPTION>
        requires ((OPTION & 1) == CLI_OPTION_REQUIRED && (OPTION & 2) == 0)
        void cleanup(std::vector<std::exception*>* exceptions) {
            exceptions->push_back(new std::invalid_argument("No arg for thing"));
        }

        template <typename U = UnderlyingType>
        requires std::is_same_v<U, bool>
        void initializeVars() {
            t = false;
            isSet = true;
        }

        template <typename U = UnderlyingType>
        requires (!std::is_same_v<U, bool>)
        void initializeVars() {
        }

        void internalParse(const char* const slice) {
            parse(slice);
            isSet = true;
        }

        template <typename U = UnderlyingType>
        requires (std::is_integral_v<U> && !std::is_same_v<U, bool>)
        void parse (const char* const slice) {
            t = static_cast<int>(strtol(slice, nullptr, 10));
        }

        template <typename U = UnderlyingType>
        requires std::is_same_v<U, bool>
        void parse (const char* const slice) {
            t = true;
        }

        template <typename U = UnderlyingType>
        requires std::is_class_v<U>
        void parse (const char* const slice) {
            t = UnderlyingType(slice);
        }

        constexpr bool requiresArgument() {
            return !std::is_same_v<UnderlyingType, bool>;
        }

        size_t numberOfCodes = 0;
        char** code;
        bool isSet = false;
        UnderlyingType t;
        char* name;
        char* description;
    };

    static bool startsWithPrefix(const char* const argv, size_t* indexOfPrefix) {
        for (size_t i =0; i< NUMBER_OF_PREFIXES; i++) {
            if (strncmp(prefixes[i], argv, strlen(prefixes[i])) == 0) {
                *indexOfPrefix = i;
                return true;
            }
        }
        return false;
    }
//    static bool startsWithPrefix(const char* const argv) {
//        for (size_t i =0; i< NUMBER_OF_PREFIXES; i++) {
//            if (strncmp(prefixes[i], argv, strlen(prefixes[i])) == 0) {
//                return true;
//            }
//        }
//        return false;
//    }

#define CLI_VERIFY_IS_SET() \
    ([&] { \
        options.cleanup(nullptr);\
    }(), ...); \

    template<OptType... Opts>
    void parse(int argc, const char* const* argv, Opts&... options) {
        std::qsort( // make sure prefixes are in order of size
                prefixes,
                NUMBER_OF_PREFIXES,
                sizeof(typeof(prefixes[0])),
                [] (const void* x, const void* y) {
                    const char* const xPrefix = *static_cast<const char* const*>(x);
                    const char* const yPrefix = *static_cast<const char* const*>(y);
                    const auto cmp = strlen(yPrefix) <=> strlen(xPrefix);
                    if (cmp < nullptr) {
                        return -1;
                    } else if (cmp > nullptr) {
                        return 1;
                    }
                    return 0;
                }
        );

        int arg = 1;
        while (arg < argc) {
            size_t i = 0;
            if (startsWithPrefix(argv[arg], &i)) {
                ([&] {
                    if (arg >= argc) return; // toasted loop
                    size_t sizeOfPrefix;
                    if (options.accepts(&argv[arg][strlen(prefixes[i])], &sizeOfPrefix)) {
                        if (sizeOfPrefix + strlen(prefixes[i]) < strlen(argv[arg])) {
                            options.parse(&argv[arg][sizeOfPrefix + strlen(prefixes[i])]);
                            arg++;
                        } else {
                            size_t indexOfPrefix;
                            if (!options.requiresArgument()) { // is bool and at end of list
//                                if ()
                                // todo make check for any arguments, because if they exist then we should just crash or something
                                options.internalParse("");
                                arg++;
                            } else if (startsWithPrefix(argv[arg + 1], &indexOfPrefix)) { // make sure next prefix isn't an actual instruction
                                bool valid = false;
                                ([&] {
                                    if (options.accepts(&argv[arg + 1][strlen(prefixes[indexOfPrefix])])) valid = true;
                                }(), ...);
                                if (!valid) goto isNotValidPrefix;
                                // todo this should be an error if it gets here, basically means we're missing a valid argument
                            } else if (options.requiresArgument()){
                                isNotValidPrefix:
                                options.internalParse(argv[arg + 1]);
                                arg += 2;
                            } else {
                                // error, argument given to option that does not require arguments
                            }
                        }
                    }
                }(), ...);
            } else {
                argc++;
            }
        }
//        CLI_VERIFY_IS_SET();
    }

    template<OptType... Opts>
    void parse(int argc, const char* const* argv, std::vector<const char*>* defaults, Opts&... options);
}

#endif //COMMANDLINEPARSER_CLIPARSER_HPP
