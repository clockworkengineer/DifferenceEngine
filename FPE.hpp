#ifndef FPE_HPP
#define FPE_HPP

namespace FPE {

    //
    // Option map command indexes
    //
    
    constexpr char const *kConfigOption{"config"};
    constexpr char const *kWatchOption{"watch"};
    constexpr char const *kDestinationOption{"destination"};
    constexpr char const *kTaskOption{"task"};
    constexpr char const *kCommandOption{"command"};
    constexpr char const *kMaxDepthOption{"maxdepth"};
    constexpr char const *kExtensionOption{"extension"};
    constexpr char const *kQuietOption{"quiet"};
    constexpr char const *kDeleteOption{"delete"};
    constexpr char const *kLogOption{"log"};
    constexpr char const *kSingleOption{"single"};
    constexpr char const *kKillCountOption{"killcount"};
    constexpr char const *kServerOption{"server"};
    constexpr char const *kUserOption{"user"};
    constexpr char const *kPasswordOption{"password"};
    constexpr char const *kRecipientOption{"recipient"};
    constexpr char const *kMailBoxOption{"mailbox"};
    constexpr char const *kArchiveOption{"archive"};
    constexpr char const *kDatabaseOption{"database"};
    constexpr char const *kCollectionOption{"collection"};
    constexpr char const *kListOption{"list"};

    //
    // File Processing Engine.
    //
    
    void FileProcessingEngine(int argc, char** argv);
    
} // namespace FPE 

#endif /* FPE_HPP */
