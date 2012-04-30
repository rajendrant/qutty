#include "TmuxLayout.h"

bool TmuxLayout::parseLayoutChildren(istringstream &iresp)
{
    for (int i=0; ; i++) {
        TmuxLayout tchild;
        tchild.parseLayout(iresp);
        char ch = iresp.get();
        if (ch != ']' && ch != '}' && ch != ',')
            goto cu0;
        child.push_back(tchild);
        tchild.parent = this;
        if (ch == ']' || ch == '}')
            break;
    }
    return true;
cu0:
    return false;
}


bool TmuxLayout::parseLayout(istringstream &iresp)
{
    int no;
    iresp>>width;
    if (iresp.get()!='x') {
        goto cu0;
    }
    iresp>>height;
    if (iresp.get()!=',') {
        goto cu0;
    }
    iresp>>x;
    if (iresp.get()!=',') {
        goto cu0;
    }
    iresp>>y;
    //cout<<this<<" Layout "<<width<<"  "<<height<<"  "<<x<<"  "<<y<<endl;
    if (iresp.peek()=='{' || iresp.peek()=='[') {
        char openChar = iresp.get();
        layoutType = (openChar == '{') ?
                    TmuxLayout::TMUX_LAYOUT_TYPE_VERTICAL :
                    TmuxLayout::TMUX_LAYOUT_TYPE_HORIZONTAL;
        if (!parseLayoutChildren(iresp))
            goto cu0;
    } else {
        if (iresp.get()!=',')
            goto cu0;
        iresp>>paneid;
    }
    return true;
cu0:
    return false;
}

string TmuxLayout::dumpLayout()
{
    ostringstream ret(ostringstream::out);
    ret<<width<<"x"<<height<<","<<x<<","<<y;
    switch (layoutType) {
      case TmuxLayout::TMUX_LAYOUT_TYPE_NONE:
        ret<<","<<paneid;
        break;
      case TmuxLayout::TMUX_LAYOUT_TYPE_HORIZONTAL:
      case TmuxLayout::TMUX_LAYOUT_TYPE_VERTICAL:
        ret << (layoutType==TmuxLayout::TMUX_LAYOUT_TYPE_VERTICAL ? '{' : '[');
        for (int i=0; i<child.size(); i++) {
            ret<<child[i].dumpLayout();
            if (i != child.size()-1)
                ret<<",";
        }
        ret << (layoutType==TmuxLayout::TMUX_LAYOUT_TYPE_VERTICAL ? '}' : ']');
    }
    return ret.str();
}

#ifdef TMUX_LAYOUT_PARSER_TEST
int main()
{
    const char *test[] = {
        "78x28,0,0,0",
        "114x36,0,0,0",
        "78x28,0,0[78x14,0,0,1,78x13,0,15{39x13,0,15,2,38x13,40,15,3}]",
        "114x36,0,0[114x18,0,0,1,114x17,0,19{57x17,0,19,2,56x17,58,19,3}]",
        "204x50,0,0[204x25,0,0,204x12,0,26,204x11,0,39{102x11,0,39,101x11,103,39[101x5,103,39,101x5,103,45{50x5,103,45,50x5,154,45}]}]",
        "114x36,0,0[114x9,0,0,1,114x4,0,10,7,114x3,0,15,8,114x17,0,19{57x17,0,19,2,56x17,58,19,3}]",
        "114x36,0,0{57x36,0,0[57x18,0,0,4,57x17,0,19,6],56x36,58,0,5}",
        "114x36,0,0[114x9,0,0,1,114x4,0,10,7,114x3,0,15,8,114x17,0,19{57x17,0,19,2,56x17,58,19,3}]",
        "114x36,0,0[114x9,0,0,1,114x8,0,10,7,114x17,0,19{57x17,0,19,2,56x17,58,19,3}]",
    };
    for (int i=0; i<sizeof(test)/sizeof(test[0]); i++) {
        TmuxLayout layout;
        string tt(test[i]);
        cout<<"Test input "<<i<<" \""<<test[i]<<"\""<<endl;
        if (layout.initLayout(tt)) {
            string res = layout.dumpLayout();
            if (res != test[i]) {
                cout<<"Test not matching\n";
                cout<<res<<endl;
                cout<<test[i]<<endl;
            }
        }
        cout<<endl;
    }
}
#endif
