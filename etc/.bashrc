# .bashrc
#################################################################
#
#         .bashrc file
#
#################################################################
#
# User specific aliases and functions

# Source global definitions
if [ -f /etc/bashrc ]; then
    . /etc/bashrc
fi

#
# set up search path
PATH=/opt/conda/bin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/X11R6/bin

#
# set this for all shells
ulimit -c 0 -s unlimited
set -o noclobber
umask 077
#stty -ixon

#
# aliases for all shells
alias cp='cp -i'
alias mv='mv -i'
alias rm='rm -i'

#
#
# set Shell Variable
IGNOREEOF=100
HISTSIZE=500
HISTFILESIZE=500

#
# set Prompt
if [ "$USER" = "root" ]; then
    PS1="#"
else
    PS1="$"
fi
PS1=$"[\\u@\\h \\W]:\!$PS1 "

#
# other aliases
alias ls='ls -F --color=auto'
alias eng='LANG=C LANGUAGE=C LC_ALL=C'
alias hist='history | tail -20'
alias cls=clear
alias bd=popd
alias pd=pushd
alias vi='vim'


# >>> conda initialize >>>
CONDADIR=/opt/conda
if [ -d /usr/local/anaconda ]; then
    CONDADIR=/usr/local/anaconda
fi

# !! Contents within this block are managed by 'conda init' !!
#__conda_setup="$(${CONDADIR}/bin/conda 'shell.bash' 'hook' 2> /dev/null)"
#if [ $? -eq 0 ]; then
#    eval "$__conda_setup"
#else
    if [ -f $CONDADIR"/etc/profile.d/conda.sh" ]; then
        . $CONDADIR"/etc/profile.d/conda.sh"
    else
        export PATH=$CONDADIR"/bin:$PATH"
    fi
#fi
#unset __conda_setup
# <<< conda initialize <<<
