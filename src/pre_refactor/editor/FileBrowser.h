//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "pch.h"

namespace tri {

    class FileBrowser {
    public:
        FileBrowser();

        int getFileAssociation(const std::string &extension);
        void openBrowseWindow(const std::string &buttonName, const std::string &windowName, int fileTypeId, const std::function<void(const std::string &)> &selectCallback);
        void browse(const std::function<void(const std::string &, int)> &fileMenuCallback = nullptr);

        void update();
        void startup();

    private:
        void directoryMenu(const std::string &directory);
        void newFileBox(const std::string &directory);

        std::unordered_map<std::string , int> associations;
        std::string buttonName;
        std::string windowName;
        int fileTypeId;
        bool openWindow;
        bool canSelectFiles;
        std::string selectedFile;
        std::string newFileDirectory;
        bool newFileIsDirectory;
        std::string inputBuffer;
        std::function<void(const std::string &)> selectCallback;
        std::function<void(const std::string &, int)> fileMenuCallback;

        class DirectoryNode {
        public:
            std::string name;
            std::string path;
            std::string extension;
            bool isFile;
            bool shouldUpdate;
            std::vector<DirectoryNode> directories;
            std::vector<DirectoryNode> files;

            DirectoryNode(const std::string &path = "");
            void update();
        };
        DirectoryNode rootNode;
        DirectoryNode updateRootNode;
        std::mutex mutex;
        bool updatingTree;

        void browse(DirectoryNode& node);
    };

}
