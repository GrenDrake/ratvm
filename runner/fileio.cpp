#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include "textutil.h"
#include "gamedata.h"


static std::string getRealPath(const std::string &filename) {
    std::string basePath;

    if (basePath.empty()) {
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        std::string path = homedir;
        if (path.back() != '/') path += '/';
        basePath = path;
#else
        basePath = "./";
#endif
    }

    return basePath + filename;
}

static int read16(std::istream &inf);
static int read32(std::istream &inf);
static std::string readString(std::istream &inf);
static void write16(std::ostream &inf, int value);
static void write32(std::ostream &inf, int value);
static void writeString(std::ostream &inf, const std::string &text);

static int read16(std::istream &inf) {
    uint16_t value;
    inf.read(reinterpret_cast<char*>(&value), 2);
    return value;
}

static int read32(std::istream &inf) {
    uint32_t value;
    inf.read(reinterpret_cast<char*>(&value), 4);
    return value;
}

static std::string readString(std::istream &inf) {
    int len = read16(inf);
    char *buffer = new char[len + 1];
    inf.read(buffer, len);
    buffer[len] = 0;
    std::string result = buffer;
    delete buffer;
    return result;
}

static void write16(std::ostream &inf, int value) {
    uint16_t sized = value;
    inf.write(reinterpret_cast<char*>(&sized), 2);
}

static void write32(std::ostream &inf, int value) {
    uint32_t sized = value;
    inf.write(reinterpret_cast<char*>(&sized), 4);
}

static void writeString(std::ostream &inf, const std::string &text) {
    write16(inf, text.size());
    inf.write(text.c_str(), text.size());
}





FileList GameData::getFileList() {
    FileList list;
    std::ifstream listfile(getRealPath("ratvm.lst"));
    if (!listfile) return list;

    unsigned recordCount = read32(listfile);
    for (unsigned i = 0; i < recordCount; ++i) {
        FileRecord record;
        uint32_t fileId = 0;
        uint32_t timestamp = 0;

        listfile.read(reinterpret_cast<char*>(&fileId), sizeof(fileId));
        record.fileId = fileId;

        record.name = readString(listfile);

        listfile.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        record.date = timestamp;

        record.gameId = readString(listfile);

        list.push_back(record);
    }
    return list;
}

bool GameData::saveFileList(const FileList &files) {
    std::ofstream out(getRealPath("ratvm.lst"));
    if (!out) return false;

    write32(out, files.size());
    for (const FileRecord &file : files) {
        write32(out, file.fileId);
        writeString(out, file.name);
        write32(out, file.date);
        writeString(out, getString(refGameid).text);
    }
    return true;
}

Value GameData::getFile(const std::string &fileName) {
    FileList files = getFileList();

    FileRecord file;
    file.fileId = -1;

    for (FileRecord &record : files) {
        if (record.name == fileName) {
            file = record;
            break;
        }
    }

    if (file.fileId < 0) {
        return noneValue;
    }

    Value newListId = makeNew(Value::List);
    if (newListId.type != Value::List) {
        throw GameError("Failed to create list for new file.");
    }
    ListDef &newList = getList(newListId.value);
    std::stringstream realFilename;
    realFilename << "rat" << std::setfill('0') << std::setw(5) << file.fileId << ".fil";
    std::ifstream inf(getRealPath(realFilename.str()));
    while (1) {
        int v = read32(inf);
        if (inf.eof()) break;
        newList.items.push_back(Value(Value::Integer, v));
    }

    return newListId;
}

int nextFile(const FileList &fileList) {
    int next = 1;
    for (const FileRecord &file : fileList) {
        if (file.fileId >= next) next = file.fileId + 1;
    }
    return next;
}


bool GameData::saveFile(const std::string &filename, const ListDef *list) {
    FileList files = getFileList();

    FileRecord file;
    file.fileId = -1;

    for (FileRecord &record : files) {
        if (record.name == filename) {
            record.date = time(nullptr);
            file = record;
            break;
        }
    }

    if (file.fileId < 0) {
        file.fileId = nextFile(files);
        file.name = filename;
        file.date = time(nullptr);
        files.push_back(file);
    }

    saveFileList(files);


    std::stringstream realFilename;
    realFilename << "rat" << std::setfill('0') << std::setw(5) << file.fileId << ".fil";
    std::ofstream out(getRealPath(realFilename.str()));
    for (const Value &v : list->items) {
        if (v.type != Value::Integer) {
            throw GameError("List of data to save must contain only integers; file data corrupted.");
        }
        write32(out, v.value);
    }

    return true;
}

bool GameData::deleteFile(const std::string &filename) {
    FileList files = getFileList();

    FileRecord file;
    file.fileId = -1;

    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        if (iter->name == filename) {
            file = *iter;
            files.erase(iter);
            break;
        }
    }
    if (file.fileId < 0) return false;
    saveFileList(files);

    std::stringstream realFilename;
    realFilename << "rat" << std::setfill('0') << std::setw(5) << file.fileId << ".fil";
    std::remove(getRealPath(realFilename.str()).c_str());
    return true;
}