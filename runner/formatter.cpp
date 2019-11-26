#include <algorithm>
#include <iostream>
#include <cctype>
#include <string>
#include <vector>

#include "formatter.h"
#include "textutil.h"


struct TagInfo {
    std::string name;
    bool hasContent;
    bool topLevel;
    bool hasAttributes;
};

struct TextToken {
    enum Type { Text, Tag, EndTag, Paragraph, End };
    Type type;
    std::string text;
};

struct TextNode {
    ~TextNode() {
        for (TextNode *n : children) delete n;
    }

    enum Type { Root, Text, Tag, Paragraph };
    Type type;
    std::string text;
    const TagInfo *tagInfo;
    std::vector<std::string> attributes;
    std::vector<TextNode*> children;

    void add(TextNode *node) {
        if (node) children.push_back(node);
    }
    void addParagraph() {
        if (children.empty()) return;
        if (children.back()->type == TextNode::Paragraph) return;
        TextNode *text = new TextNode;
        text->type = TextNode::Paragraph;
        children.push_back(text);
    }
};

struct TextBuildState {
    const std::vector<TextToken> &tokens;
    std::vector<TextToken>::size_type pos;

    static const TextToken endToken;
    bool end() const {
        return pos >= tokens.size();
    }
    const TextToken&  here() const {
        if (end()) return endToken;
        return tokens[pos];
    }
    void advance() {
        if (!end()) ++pos;
    }
};
const TextToken TextBuildState::endToken{TextToken::End};

struct TextParseState {
    const std::string &text;
    std::string::size_type pos;

    bool end() const {
        return pos >= text.size();
    }
    char here() const {
        if (end()) return 0;
        return text[pos];
    }
    void advance() {
        if (!end()) ++pos;
    }
};


static std::vector<TextToken> parseSourceText(const std::string &text, ParseResult &results);
static TextNode* buildParseTree(const std::vector<TextToken> &tokens, ParseResult &results);
static std::string formatAsText(TextNode *n, ParseResult &results, std::vector<std::string> &formats);


TagInfo tags[] = {
    { "b",      true,   false,  false },
    { "br",     false,  false,  false },
    { "hr",     false,  true,   false  },
    { "i",      true,   false,  false },
    { "color",  true,   false,  true },
};
TagInfo badTag = { "", false };


const TagInfo &getTagInfo(const std::string &tag) {
    for (const TagInfo &t : tags) {
        if (t.name == tag) return t;
    }
    return badTag;
}

std::vector<std::string> explodeString(const std::string &s) {
    std::vector<std::string> parts;
    const char *whitespace = " \t\n\r";
    std::string::size_type p = 0, n = 0;

    if (s.empty()) return parts;

    p = s.find_first_not_of(whitespace);

    while (n != std::string::npos) {
        while (p < s.size() && c_isspace(s[p])) ++p;
        if (p >= s.size()) break;
        n = s.find_first_of(whitespace, p);
        std::string sub = s.substr(p, n - p);
        if (!sub.empty()) {
            parts.push_back(sub);
        }
        p = n + 1;
    }

    return parts;
}



std::vector<TextToken> parseSourceText(const std::string &text, ParseResult &results) {
    TextParseState state{text};
    std::vector<TextToken> nodes;
    if (state.end()) return nodes;

    while (!state.end()) {
        if (state.here() == '[') {
            state.advance();
            std::string::size_type start = state.pos;
            while (!state.end() &&  state.here() != ']') {
                state.advance();
            }
            std::string::size_type end = state.pos;
            state.advance();
            TextToken node;
            node.text = state.text.substr(start, end - start);
            if (node.text[0] == '/')    node.type = TextToken::EndTag;
            else                        node.type = TextToken::Tag;
            nodes.push_back(node);
        } else if (state.here() == '\n') {
            state.advance();
            nodes.push_back(TextToken{TextToken::Paragraph});
        } else {
            std::string::size_type start = state.pos;
            while (!state.end() && state.here() != '\n' && state.here() != '[') {
                state.advance();
            }
            std::string::size_type end = state.pos;
            TextToken node;
            node.text = state.text.substr(start, end - start);
            for (char &c : node.text) {
                if (c == '\t') c = ' ';
                if (c == '\r') c = '\n';
            }
            node.type = TextToken::Text;
            nodes.push_back(node);
        }
    }

    return nodes;
};


TextNode* buildParseTree(const std::vector<TextToken> &tokens, ParseResult &results) {
    TextBuildState state{tokens};
    TextNode *root = new TextNode;
    root->type = TextNode::Root;

    std::vector<TextNode*> parent;
    parent.push_back(root);
    std::vector<TextNode*> tags;

    while (!state.end()) {
        if (state.here().type == TextToken::Text) {
            TextNode *text = new TextNode;
            text->type = TextNode::Text;
            text->text = state.here().text;
            parent.back()->add(text);
        } else if (state.here().type == TextToken::Paragraph) {
            if (!tags.empty()) results.addError("Paragraph break may only occur at top level.");
            parent.back()->addParagraph();
        } else if (state.here().type == TextToken::Tag) {
            auto parts = explodeString(state.here().text);
            if (!parts.empty()) {
                const TagInfo &tag = getTagInfo(parts[0]);
                if (!tag.name.empty()) {
                    if (tag.topLevel) {
                        if (!tags.empty()) {
                            results.addError("Tag " + tag.name + " may only occur at top level.");
                        }
                        parent.back()->addParagraph();
                    }

                    TextNode *tagNode = new TextNode;
                    tagNode->type = TextNode::Tag;
                    tagNode->text = tag.name;
                    if (parts.size() > 1) {
                        if (tag.hasAttributes) {
                            parts.erase(parts.begin());
                            tagNode->attributes = parts;
                        } else {
                            results.addError("Tag " + tag.name + " does not take attributes.");
                        }
                    }
                    parent.back()->add(tagNode);
                    tagNode->tagInfo = &tag;

                    if (tag.topLevel) {
                        parent.back()->addParagraph();
                    }

                    if (tag.hasContent) {
                        tags.push_back(tagNode);
                        parent.push_back(tagNode);
                    }
                } else {
                    results.addError("Unknown tag " + parts[0] + ".");
                }
            } else {
                results.addError("Empty tag name.");
            }
        } else if (state.here().type == TextToken::EndTag) {
            if (tags.empty()) results.addError("Closing tag with no opened tags.");
            else {
                std::string tagName = state.here().text.substr(1);
                if (tagName != tags.back()->text) {
                    results.addError("Closing tag " + tagName + " does not match opening tag " + tags.back()->text + ".");
                } else {
                    parent.pop_back();
                    tags.pop_back();
                }
            }
        }
        state.advance();
    }

    if (!tags.empty()) {
        for (const TextNode *tag : tags) {
            results.addError("Tag " + tag->text + " not closed.");
        }
    }
    return root;
}

void applyFormats(std::string &text, std::vector<std::string> &formats) {
    for (const std::string s : formats) text += s;
}

std::string formatAsText(TextNode *n, ParseResult &results, std::vector<std::string> &formats) {
    std::string result;
    if (!n) {
        results.addError("Null parse tree.");
        return "";
    }

    switch(n->type) {
        case TextNode::Root:
            if (n->children.empty()) break;
            while (n->children.back()->type == TextNode::Paragraph) {
                n->children.erase(n->children.end() - 1);
            }
            break;
        case TextNode::Paragraph:
            result += "\n\n";
            break;
        case TextNode::Text:
            result += n->text;
            break;
        case TextNode::Tag:
            if (n->text == "br") result += "\n";
            if (n->text == "hr") result += "--------------------------------------------------";
            if (n->text == "b") {
                const std::string code = "\x1b[1m";
                formats.push_back(code);
                result += code;
            }
            if (n->text == "i") {
                const std::string code = "\x1b[4m";
                formats.push_back(code);
                result += code;
            }
            if (n->text == "color") {
                if (n->attributes.empty()) results.addError("Color tag requires name of color.");
                else if (n->attributes.size() > 1) results.addError("Too many arguments to color tag.");
                else {
                    std::string code;
                    if (n->attributes[0] == "red") code = "\x1b[31m";
                    else if (n->attributes[0] == "green") code = "\x1b[32m";
                    else if (n->attributes[0] == "yellow") code = "\x1b[33m";
                    else if (n->attributes[0] == "blue") code = "\x1b[34m";
                    else if (n->attributes[0] == "magenta") code = "\x1b[35m";
                    else if (n->attributes[0] == "cyan") code = "\x1b[36m";
                    else if (n->attributes[0] == "default") code = "\x1b[37m";
                    else results.addError("Unrecognized colour name " + n->attributes[0] + ".");
                    formats.push_back(code);
                    result += code;
                }
            }
            break;
    }

    for (TextNode *c : n->children) {
        result += formatAsText(c, results, formats);
    }

    if (n->type == TextNode::Tag) {
        if (n->text == "b" || n->text == "i" || n->text == "color") {
            result += "\x1b[0m";
            formats.pop_back();
            applyFormats(result, formats);
        }
    }

    return result;
}

ParseResult formatText(const std::string &text) {
    ParseResult results;
    auto nodes = parseSourceText(text, results);
    TextNode *root = buildParseTree(nodes, results);
    if (!root) {
        results.addError("Failed to build parse tree.");
        return results;
    }
    std::vector<std::string> formats;
    results.finalResult = formatAsText(root, results, formats);
    delete root;
    return results;
}