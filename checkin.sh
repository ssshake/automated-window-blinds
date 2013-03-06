#!/bin/bash
git config --global user.name "shake"
git config --global user.email git@doobiest.net
git add .
git commit -m "$1"
git push origin master
