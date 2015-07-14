#include <SFML/System.hpp>
#include <dirent.h>
#include <stdio.h>
#include "resources.h"

PVector<ResourceProvider> resourceProviders;

ResourceProvider::ResourceProvider()
{
    resourceProviders.push_back(this);
}

bool ResourceProvider::searchMatch(const string name, const string searchPattern)
{
    std::vector<string> parts = searchPattern.split("*");
    int pos = 0;
    if (parts[0].length() > 0)
    {
        if (name.find(parts[0]) != 0)
            return false;
    }
    for(unsigned int n=1; n<parts.size(); n++)
    {
        int offset = name.find(parts[n], pos);
        if (offset < 0)
            return false;
        pos = offset + parts[n].length();
    }
    return pos == int(name.length());
}

string ResourceStream::readLine()
{
    string ret;
    char c;
    while(true)
    {
        if (read(&c, 1) < 1)
            return ret;
        if (c == '\n')
            return ret;
        ret += string(c);
    }
}

class FileResourceStream : public ResourceStream
{
    FILE* f;
public:
    FileResourceStream(FILE* f)
    : f(f)
    {
    }
    virtual ~FileResourceStream()
    {
        fclose(f);
    }
    
    virtual sf::Int64 read(void* data, sf::Int64 size)
    {
        return fread(data, 1, size, f);
    }
    virtual sf::Int64 seek(sf::Int64 position)
    {
        fseek(f, position, SEEK_SET);
        return position;
    }
    virtual sf::Int64 tell()
    {
        return ftell(f);
    }
    virtual sf::Int64 getSize()
    {
        fpos_t pos;
        fgetpos(f, &pos);
        fseek(f, 0, SEEK_END);
        sf::Int64 size = ftell(f);
        fsetpos(f, &pos);
        return size;
    }
};


DirectoryResourceProvider::DirectoryResourceProvider(string basepath)
: basepath(basepath)
{
}

P<ResourceStream> DirectoryResourceProvider::getResourceStream(string filename)
{
    FILE* f = fopen((basepath + filename).c_str(), "rb");
    if (f)
        return new FileResourceStream(f);
    return NULL;
}

std::vector<string> DirectoryResourceProvider::findResources(string searchPattern)
{
    std::vector<string> found_files;
    findResources(found_files, "", searchPattern);
    return found_files;
}

void DirectoryResourceProvider::findResources(std::vector<string>& found_files, const string path, const string searchPattern)
{
    DIR* dir = opendir((basepath + path).c_str());
    if (!dir)
        return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        string name = path + string(entry->d_name);
        if (searchMatch(name, searchPattern))
            found_files.push_back(name);
        findResources(found_files, path + string(entry->d_name) + "/", searchPattern);
    }
    closedir(dir);
}

P<ResourceStream> getResourceStream(string filename)
{
    foreach(ResourceProvider, rp, resourceProviders)
    {
        P<ResourceStream> stream = rp->getResourceStream(filename);
        if (stream)
            return stream;
    }
    return NULL;
}

std::vector<string> findResources(string searchPattern)
{
    std::vector<string> foundFiles;
    foreach(ResourceProvider, rp, resourceProviders)
    {
        std::vector<string> res = rp->findResources(searchPattern);
        foundFiles.insert(foundFiles.end(), res.begin(), res.end());
    }
    return foundFiles;
}
