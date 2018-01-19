#pragma once

#include <ostream>
#include <iostream>

namespace ANNESE {
#define Log(Traits) TeeLog::Instance<Traits>() << Traits::Instance().name() << ":" << MY_FILENAME << ":" << __LINE__ << ": "

    class Traits {
    public:
        virtual std::string name() = 0;

        virtual bool shouldWrite() = 0;
    };

    class Error : public Traits {
    public:
        static Error &Instance() {
            static Error error;
            return error;
        }

        std::string name() override {
            return "Error";
        }

        bool shouldWrite() override {
            return true;
        }

    private:
        Error() = default;
    };

    class Info : public Traits {
    public:
        static Info &Instance() {
            static Info info;
            return info;
        }

        std::string name() override {
            return "Info";
        }

        bool shouldWrite() override {
            return true;
        }

    private:
        Info() = default;
    };

    class Debug : public Traits {
    public:
        static Debug &Instance() {
            static Debug debug;
            return debug;
        }

        std::string name() override {
            return "Debug";
        }

        bool shouldWrite() override {
#if DEBUG
            return true;
#else
            return false;
#endif
        }

    private:
        Debug() = default;
    };

    class TeeLog {
    public:
        template<typename Traits = Info>
        static TeeLog &Instance() {
            return Instance(&Traits::Instance());
        }

        using OstreamType = std::basic_ostream<char, std::char_traits<char>>;

        using StdEndl = OstreamType &(*)(OstreamType&);

        TeeLog &operator<<(StdEndl manip) {
            if (!mTraits->shouldWrite()) {
                return *this;
            }
            if (mLogFile) {
                *mLogFile << manip;
                mLogFile->flush();
            }
            if (mWriteToStandardOutput) {
                std::cout << manip;
            }
            return *this;
        }

        template<typename T>
        TeeLog &operator<<(const T &value) {
            if (!mTraits->shouldWrite()) {
                return *this;
            }
            if (mLogFile) {
                *mLogFile << value;
            }
            if (mWriteToStandardOutput) {
                std::cout << value;
            }
            return *this;
        }

        void setLogFile(std::unique_ptr<std::ostream> &&logFile) {
            mLogFile = std::move(logFile);
        }

        void setWriteToStandardOutput(bool value) {
            mWriteToStandardOutput = true;
        }

    private:
        TeeLog() = default;

        static TeeLog &Instance(Traits *traits) {
            static TeeLog log;
            log.mTraits = traits;
            return log;
        }

        Traits *mTraits = nullptr;

        std::unique_ptr<std::ostream> mLogFile;

        bool mWriteToStandardOutput = true;
    };
}