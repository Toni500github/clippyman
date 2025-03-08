#include <getopt.h>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <thread>

#include "EventData.hpp"
#include "fmt/base.h"
#include "fmt/os.h"
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

void CopyCallback(const CopyEvent& event)
{
    info("Copied: {}", event.content);
}

void CopyEntry(const CopyEvent& event)
{
    FILE* file = fopen("/tmp/test.json", "r+");
    rapidjson::Document doc;
    char buf[UINT16_MAX] = {0};
    rapidjson::FileReadStream stream(file, buf, sizeof(buf));

    if (doc.ParseStream(stream).HasParseError())
    {
        fclose(file);
        die("Failed to parse /tmp/test.json: {} at offset {}", rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value key("content", allocator);
    rapidjson::Value value(event.content.c_str(), allocator);
    if (doc.HasMember(event.index.data()) && doc[event.index.data()].IsObject())
        doc[event.index.data()].AddMember(key, value, allocator);
    else
        doc["Other"].AddMember(key, value, allocator);

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

void CreateInitialCache(const std::string_view path)
{
    if (access(path.data(), F_OK) == 0)
        return;

    constexpr std::string_view json = R"({
    "A": {},
    "B": {},
    "C": {},
    "D": {},
    "E": {},
    "F": {},
    "G": {},
    "H": {},
    "I": {},
    "J": {},
    "K": {},
    "L": {},
    "M": {},
    "N": {},
    "O": {},
    "P": {},
    "Q": {},
    "R": {},
    "S": {},
    "T": {},
    "U": {},
    "V": {},
    "W": {},
    "X": {},
    "Y": {},
    "Z": {},
    "Other": {}
})";
    auto f = fmt::output_file(path.data(), fmt::file::CREATE | fmt::file::RDWR | fmt::file::TRUNC);
    f.print("{}", json);
    f.close();
    //exit(0);
}

bool input = false;

bool parseargs(int argc, char* argv[])
{
    int opt = 0;
    int option_index = 0;
    opterr = 1; // re-enable since before we disabled for "invalid option" error
    const char *optstring = "-Vhi";

    static const struct option opts[] = {
        {"input", no_argument, 0, 'i'},
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
                input = true; break;

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

    CreateInitialCache("/tmp/test.json");

    if (!parseargs(argc, argv))
        return EXIT_FAILURE;

    bool piped = !isatty(STDIN_FILENO);
    debug("piped = {} && input = {}", piped, input);
    if (piped || input)
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
