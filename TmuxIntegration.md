# Introduction #

http://code.google.com/p/iterm2/wiki/TmuxIntegration

# Building Tmux #

First get the tmux2 repository's 'command\_mode' branch.
```
git clone https://github.com/gnachman/tmux2.git
git checkout -t origin/command_mode
```

Now build as follows:
```
cd tmux2
sh autogen.sh
./configure
make
```