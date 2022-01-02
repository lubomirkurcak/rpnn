@echo off
git pull
git add -A --ignore-errors
git commit -m "%*"
git push
