#include <getopt.h>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <thread>

#include "config.hpp"
#include "EventData.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/os.h"
#include "rapidjson/rapidjson.h"
#include "util.hpp"

#include "clipboard/unix/ClipboardListenerUnix.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#if PLATFORM_XORG
# include "clipboard/x11/ClipboardListenerX11.hpp"
#elif PLATFORM_WAYLAND
# include "clipboard/wayland/ClipboardListenerWayland.hpp"
#endif

Config config;

void CopyCallback(const CopyEvent& event)
{
    info("Copied: {}", event.content);
}

void CopyEntry(const CopyEvent& event)
{
    FILE* file = fopen(config.path.c_str(), "r+");
    rapidjson::Document doc;
    char buf[UINT16_MAX] = {0};
    rapidjson::FileReadStream stream(file, buf, sizeof(buf));

    if (doc.ParseStream(stream).HasParseError())
    {
        fclose(file);
        die("Failed to parse {}: {} at offset {}", config.path, rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    std::string id_str;

    rapidjson::Value value_content(event.content.data(), allocator);
    if (doc["entries"].ObjectEmpty())
    {
        id_str = "0";
        doc["entries"].AddMember("0", value_content, allocator);
    }
    else
    {
        const auto& lastId = (doc["entries"].MemberEnd()-1)->name;
        id_str = fmt::to_string(std::stoi(lastId.GetString())+1);
        debug("id_str = {}", id_str);
        rapidjson::GenericStringRef<char> strRef(id_str.c_str());
        doc["entries"].AddMember(strRef, value_content, allocator);
    }

    // Add it to the index.
    for (const char &c : event.content) {
        rapidjson::Value char_value(rapidjson::kArrayType);

        /* First Rule: If there's an entry for our ID, we must increment it by 1 for each other instance.
         * The base is set to 1. So if this is the first instance of 'c', it will create a new member with the value 1.
         *
         * Like so:
         * (ID #0 has 1 instance of 'c')
         * "c": { "0": 1 }
         */
        if (char_value.HasMember(id_str.c_str())) {
            char_value[id_str.c_str()].SetInt(char_value[id_str.c_str()].GetInt() + 1);
        } else {
            rapidjson::GenericStringRef<char> name(id_str.c_str());
            char_value.AddMember(name, rapidjson::Value(1), allocator);
        }

        /* Second Rule: if the index doesn't have the character, define it like so:
         * "c": {}
         */
        rapidjson::GenericStringRef<char> name(&c);
        if (doc["index"].HasMember(&c)) {
            doc["index"].RemoveMember(&c);
        }
        doc["index"].AddMember(name, char_value, allocator);
    }

    // seek back to the beginning to overwrite
    fseek(file, 0, SEEK_SET);

    char writeBuffer[UINT16_MAX] = {0};
    rapidjson::FileWriteStream writeStream(file, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> fileWriter(writeStream);
    doc.Accept(fileWriter);

    fflush(file);
    ftruncate(fileno(file), ftell(file));
    fclose(file);
}

void CreateInitialCache(const std::string& path)
{
    if (access(path.data(), F_OK) == 0)
        return;

    constexpr std::string_view json = R"({
    "entries": {},
    "index": {}
})";
    auto f = fmt::output_file(path, fmt::file::CREATE | fmt::file::RDWR | fmt::file::TRUNC);
    f.print("{}", json);
    f.close();
    //exit(0);
}

bool parseargs(int argc, char* argv[])
{
    int opt = 0;
    int option_index = 0;
    opterr = 1; // re-enable since before we disabled for "invalid option" error
    const char *optstring = "-Vhip:";

    static const struct option opts[] = {
        {"path",  required_argument, 0, 'p'},
        {"input", no_argument,       0, 'i'},
        {0,0,0,0}
    };

    optind = 0;
    while ((opt = getopt_long(argc, argv, optstring, opts, &option_index)) != -1)
    {
        switch (opt)
        {
            case 0:
                break;

            case 'i':
                config.terminal_input = true; break;
            case 'p':
                config.path = optarg; break;

            default:
                return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
#if PLATFORM_XORG
    CClipboardListenerX11 clipboardListener;
    clipboardListener.AddCopyCallback(CopyCallback);
    clipboardListener.AddCopyCallback(CopyEntry);
#elif PLATFORM_WAYLAND
    CClipboardListenerWayland clipboardListener;
    clipboardListener.AddCopyCallback(CopyCallback);
    clipboardListener.AddCopyCallback(CopyEntry);
#endif

    if (!parseargs(argc, argv))
        return EXIT_FAILURE;

    CreateInitialCache(config.path);

    bool piped = !isatty(STDIN_FILENO);
    debug("piped = {}", piped);
    if (piped || config.terminal_input)
    {
        CClipboardListenerUnix clipboardListenerUnix;
        clipboardListenerUnix.AddCopyCallback(CopyEntry);

        if (!piped)
            fmt::println("Type the text to copy into clipboard, then press enter and CTRL+D");

        clipboardListenerUnix.PollClipboard();
        return EXIT_SUCCESS;
    }

#if !PLATFORM_UNIX
    while (true)
    {
        debug("POLLING");
        clipboardListener.PollClipboard();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif

    return EXIT_SUCCESS;
}
