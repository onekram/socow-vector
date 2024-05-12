#!/bin/bash
# Иногда преподаватели изменяют репозиторий задания, исправляя проблемы
# в задании и добавляя новые тесты. Этот скрипт позволяет взять новые
# изменения. Подробная информация:
# https://cpp-kt.github.io/course/instruction.html

set -euo pipefail
IFS=$' \t\n'

if [[ -n $(git status --untracked-files=no --porcelain) ]]; then
  echo "In order to update the repository working directory must"
  echo "be clean. Please commit, checkout or stash changes and run"
  echo "./update-repo.sh again."
  exit 1
fi

if ! git remote | grep --quiet upstream; then
  source ci-extra/set-upstream.sh
  git remote add upstream "git@github.com:$UPSTREAM_REPO.git"
fi

git checkout feedback
git pull --ff-only upstream master
git push origin
git checkout master
git rebase feedback
git push -f origin
