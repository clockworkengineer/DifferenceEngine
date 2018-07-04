/*
 * File:   FPE.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 * 
 * Copyright 2016.
 *
 */

#ifndef FPE_HPP
#define FPE_HPP

namespace FPE {

    //
    // Option map command indexes
    //
    
    constexpr const char *kConfigOption{"config"};
    constexpr const char *kWatchOption{"watch"};
    constexpr const char *kDestinationOption{"destination"};
    constexpr const char *kTaskOption{"task"};
    constexpr const char *kCommandOption{"command"};
    constexpr const char *kMaxDepthOption{"maxdepth"};
    constexpr const char *kExtensionOption{"extension"};
    constexpr const char *kQuietOption{"quiet"};
    constexpr const char *kDeleteOption{"delete"};
    constexpr const char *kLogOption{"log"};
    constexpr const char *kSingleOption{"single"};
    constexpr const char *kKillCountOption{"killcount"};
    constexpr const char *kServerOption{"server"};
    constexpr const char *kUserOption{"user"};
    constexpr const char *kPasswordOption{"password"};
    constexpr const char *kRecipientOption{"recipient"};
    constexpr const char *kMailBoxOption{"mailbox"};
    constexpr const char *kArchiveOption{"archive"};
    constexpr const char *kDatabaseOption{"database"};
    constexpr const char *kCollectionOption{"collection"};
    constexpr const char *kListOption{"list"};

    //
    // File Processing Engine.
    //
    
    void FileProcessingEngine(int argc, char** argv);
    
} // namespace FPE 

#endif /* FPE_HPP */
