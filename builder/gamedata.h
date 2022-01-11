#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "bytestream.h"
#include "origin.h"
#include "symboltable.h"
#include "token.h"
#include "value.h"
#include <map>
#include <string>
#include <vector>
#include <iosfwd>

const int firstAnonymousId = 10000000;

struct ErrorMsg {
    enum Type {
        Fatal, Error, Warning
    };
    Type type;
    Origin origin;
    std::string message;
};

struct GameProperty {
    Origin origin;
    unsigned id;
    Value value;
};

struct GameObject {
    void addProperty(const Origin &origin, unsigned id, const Value &value);
    const GameProperty* getProperty(unsigned id) const;

    Origin origin;
    std::vector<GameProperty> properties;
    std::string name;
    int globalId;
    int nameString;

    std::string parentName;
    int parentId, childId, siblingId;
};

struct GameList {
    Origin origin;
    std::vector<Value> items;
    int globalId;
};

struct GameMap {
    struct MapRow {
        Value key;
        Value value;
    };

    Origin origin;
    std::vector<MapRow> rows;
    int globalId;
};

struct FlagSet {
    Origin origin;
    int globalId;
    std::vector<Value> values;
    unsigned finalValue;
};

struct Backpatch {
    unsigned position;
    std::string name;
    Origin origin;
};

struct AsmValue;
struct AsmLabel;
struct AsmOpcode;
struct FunctionDef;
class GameData;
struct FunctionBuilder {
    void build(const AsmValue *value);
    void build(const AsmLabel *label);
    void build(const AsmOpcode *opcode);
    FunctionDef *forFunction;
    GameData &gamedata;
    std::vector<Backpatch> patches;
};

struct AsmLine {
    AsmLine(const Origin &origin)
    : mOrigin(origin)
    { }
    virtual ~AsmLine() { }
    virtual void build(FunctionBuilder &builder) const = 0;
    virtual unsigned getSize() const = 0;

    const Origin& getOrigin() const { return mOrigin; }
private:
    Origin mOrigin;
};
struct AsmLabel : public AsmLine {
    AsmLabel(const Origin &origin, const std::string &text)
    : AsmLine(origin), text(text)
    { }
    virtual ~AsmLabel() override { }
    virtual void build(FunctionBuilder &builder) const override { builder.build(this); }
    virtual unsigned getSize() const override { return 0; };

    std::string text;
};

struct AsmOpcode : public AsmLine {
    AsmOpcode(const Origin &origin, int opcode)
    : AsmLine(origin), opcode(opcode)
    { }
    virtual ~AsmOpcode() override { }
    virtual void build(FunctionBuilder &builder) const override { builder.build(this); }
    virtual unsigned getSize() const override { return 1; };

    int opcode;
};
struct AsmValue : public AsmLine {
    AsmValue(const Origin &origin, const Value &value)
    : AsmLine(origin), value(value), mSize(0)
    { }
    virtual ~AsmValue() override { }
    virtual void build(FunctionBuilder &builder) const override { builder.build(this); }
    virtual unsigned getSize() const override { return 0; };

    Value value;
    unsigned mSize;
};

struct LocalDef {
    std::string name;
    Value::Type type;
    unsigned reads;
};
struct FunctionDef {
    FunctionDef()
    : argument_count(0), local_count(0), nameString(0), codePosition(0),
      codeEndPosition(0), globalId(0), isAsm(false), nextLabel(1)
    { }
    ~FunctionDef();

    void addLabel(const Origin &origin, const std::string &label);
    void addOpcode(const Origin &origin, int opcode);
    void addValue(const Origin &origin, const Value &value);

    void addLocal(const std::string &name, Value::Type type, bool alwaysUsed = false);
    const LocalDef* getLocal(int position) const;
    LocalDef* getLocal(int position);
    const LocalDef* getLocal(const std::string &name) const;
    LocalDef* getLocal(const std::string &name);
    int getLocalNumber(const std::string &name) const;

    Origin origin;
    int argument_count;
    int local_count;
    int nameString;
    std::vector<LocalDef> locals;
    std::map<std::string, unsigned> labels;
    std::string name;
    std::vector<Token> tokens;
    unsigned codePosition, codeEndPosition;
    ByteStream code;
    int globalId;
    bool isAsm;
    std::vector<AsmLine*> asmCode;
    int nextLabel;
    std::vector<std::string> continueLabels;
    std::vector<std::string> breakLabels;
};

class GameData {
public:
    GameData();
    ~GameData();

    unsigned getPropertyId(const std::string &name);
    const std::string* getPropertyName(unsigned id) const;
    unsigned getStringId(std::string name);
    const std::string& getString(unsigned id) const;
    bool isIndirectLoop(int childId, int parentId);
    void organize();
    FunctionDef* functionByName(const std::string &name);
    int checkObjectIdents();
    unsigned getSourceFileIndex(const std::string &filename);
    void addError(const Origin &origin, ErrorMsg::Type type, const std::string &text);
    bool hasErrors() const;

    void addVocab(const std::string &word);
    int  getVocabNumber(const std::string &word) const;
    void sortVocab();

    GameObject*  objectById(int ident);
    GameList*    listById(int ident);
    GameMap*     mapById(int ident);
    FunctionDef* functionById(int ident);
    FlagSet*     flagSetById(int ident);

    int errorCount;
    std::vector<ErrorMsg> errors;
    SymbolTable symbols;
    SymbolTable defaults;
    std::vector<GameObject*> objects;
    std::vector<std::string> propertyNames;
    std::vector<std::string> stringTable;
    std::vector<GameList*> lists;
    std::vector<GameMap*> maps;
    std::vector<FunctionDef*> functions;
    std::vector<FlagSet> flagsets;
    std::vector<std::string> vocab;
    ByteStream bytecode;

    std::streampos vocabStart, stringsStart, listsStart, mapsStart, objectsStart;
    std::streampos functionsStart, bytecodeStart, fileEnd;

    int nextAnonymousId;
};

std::ostream& operator<<(std::ostream &out, const Value &property);
std::ostream& operator<<(std::ostream &out, const ErrorMsg::Type &type);

void dump_gamedata(GameData &gamedata, std::ostream &out);
void dump_asm(GameData &gamedata, std::ostream &out);
void dump_functions(GameData &gamedata, std::ostream &out, bool functionBytecode);
void dump_fullBytecode(GameData &gamedata, std::ostream &out);
void dump_ir(GameData &gamedata, std::ostream &out);
void dump_objtree(GameData &gamedata, std::ostream &out);
void dump_stringtable(GameData &gamedata, std::ostream &out);
void dump_token_list(const std::vector<Token> &tokens, std::ostream &out);

#include "textutil.h"

#endif

