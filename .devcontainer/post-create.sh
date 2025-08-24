#!/bin/bash

WORKSPACE_DIR=$1
REPO_NAME=$2

cd $WORKSPACE_DIR

# Adjust permissions as docker volumes is owned by root
sudo chown -R $(id -g):$(id -u) $WORKSPACE_DIR

# Init west workspace if not present
if [ ! -d ".west" ]; then
    west init -l $REPO_NAME
fi

# Put .clangd file in expected location by clangd
ln -s $REPO_NAME/.devcontainer/.clangd

# Fetch upstream modules and setup tools
west update

pip3 install --upgrade --requirement zephyr/scripts/requirements.txt

# Put .clang-format file in expected location by clangd
ln -s zephyr/.clang-format
