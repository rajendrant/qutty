#ifndef TMUXLAYOUT_H
#define TMUXLAYOUT_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

class TmuxLayout
{
public:
    unsigned int width;
    unsigned int height;
    unsigned int x;
    unsigned int y;
    unsigned int paneid;
    enum TmuxLayoutType {
        TMUX_LAYOUT_TYPE_NONE = -1,
        TMUX_LAYOUT_TYPE_LEAF,
        TMUX_LAYOUT_TYPE_HORIZONTAL,
        TMUX_LAYOUT_TYPE_VERTICAL
    };
    TmuxLayoutType layoutType;
    vector<TmuxLayout> child;
    TmuxLayout *parent;

public:
    TmuxLayout()
    {
        paneid = -1;
        width = height = 0;
        x = y = 0;
        parent = NULL;
        layoutType = TMUX_LAYOUT_TYPE_NONE;
    }

    bool initLayout(string layout)
    {
        istringstream iresp(layout, istringstream::in);
        return parseLayout(iresp);
    }

    string dumpLayout();

private:
    bool parseLayout(istringstream &iresp);
    bool parseLayoutChildren(istringstream &iresp);

};

#endif // TMUXLAYOUT_H
