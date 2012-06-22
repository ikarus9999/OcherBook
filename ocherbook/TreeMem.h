#ifndef OCHER_FILETREE_H
#define OCHER_FILETREE_H

/** @file Represents a simple filesystem in memory.  */

#include <list>

#include "clc/data/Buffer.h"

class TreeFile
{
public:
    TreeFile(clc::Buffer &name) : name(name) {}
    TreeFile(clc::Buffer &name, clc::Buffer &data) : name(name), data(data) {}
    clc::Buffer name;
    clc::Buffer data;
    // error
};

class TreeDirectory
{
public:
    TreeDirectory(clc::Buffer &name) : name(name) {}
    ~TreeDirectory() {
        for (std::list<TreeFile*>::const_iterator i = files.begin(); i != files.end(); i++)
            delete *i;
        for (std::list<TreeDirectory*>::const_iterator i = subdirs.begin(); i != subdirs.end(); i++)
            delete *i;
    }

    clc::Buffer name;

    std::list<TreeDirectory*> subdirs;
    std::list<TreeFile*> files;

    TreeFile* createFile(clc::Buffer &name, clc::Buffer &data) {
        TreeFile *file = getFile(name);
        if (! file) {
            file = new TreeFile(name, data);
            files.push_back(file);
        }
        return file;
    }
    TreeDirectory* createDirectory(clc::Buffer &name) {
        TreeDirectory *dir = getDirectory(name);
        if (! dir) {
            dir = new TreeDirectory(name);
            subdirs.push_back(dir);
        }
        return dir;
    }

    TreeFile* getFile(const clc::Buffer &name) const {
        for (std::list<TreeFile*>::const_iterator i = files.begin(); i != files.end(); i++) {
            if ((*i)->name == name)
                return *i;
        }
        return 0;
    }
    TreeFile* getFile(const char *name) const {
        for (std::list<TreeFile*>::const_iterator i = files.begin(); i != files.end(); i++) {
            if ((*i)->name == name)
                return *i;
        }
        return 0;
    }
    TreeDirectory* getDirectory(const clc::Buffer &name) const {
        for (std::list<TreeDirectory*>::const_iterator i = subdirs.begin(); i != subdirs.end(); i++) {
            if ((*i)->name == name)
                return *i;
        }
        return 0;
    }
    TreeFile* findFile(const char* name) const {
        const char* slash = strchr(name, '/');
        if (slash) {
            size_t len = slash - name;
            for (std::list<TreeDirectory*>::const_iterator i = subdirs.begin(); i != subdirs.end(); i++) {
                if ((*i)->name.length() == len && strncmp((*i)->name.c_str(), name, len) == 0) {
                    return (*i)->findFile(slash+1);
                }
            }
        } else {
            for (std::list<TreeFile*>::const_iterator i = files.begin(); i != files.end(); i++) {
                if ((*i)->name == name) {
                    return (*i);
                }
            }
        }
        return 0;
    }
};


#endif

