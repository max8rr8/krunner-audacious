# KDE Audacious control runner

![Example](./docs/demo.png)

This is simple(currently) that allows you to use krunner to switch between tracks. Type adcs and then start typing track name, select track and press enter track should now be selected.

## Usage

* Use `adcs <song_name>` to find song and play song by name.
  Example: `adcs never gonna` to select song containing `never gonna`

* Change audacious volume with `adcs v <number>` or `adcs vol <number>`. Delta values are supported too.
  Example: `adcs vol 100` to change volume to 100%; `adcs v -20` to make volume 20% quiter 

## Features
* Switch to different tracks
* Change volume

Planned:
* Queue tracks instead of jump
* View and remove elemnts of queue

Suggest more ideas in project issues tab.

## Build & Install
```bash
git clone https://github.com/max8rr8/krunner-audacious.git
cd krunner-audacious
mkdir build && cd build
cmake ..
make
sudo make install
```

