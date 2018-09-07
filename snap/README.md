# creating a snap for nomacs

clone nomacs
```bash
# get the source
git clone git@github.com:nomacs/nomacs.git nomacs-snap

cd nomacs-snap

# checkout nomacs plugins
git clone git@github.com:nomacs/nomacs-plugins.git ImageLounge/plugins
```

get snap & snapcraft

```bash
sudo apt-get install snap
sudo snap install snapcraft
```

build the snap & test it
```bash
# in the nomacs-snap directory:
snapcraft

sudo snap install nomacs_<SNAP_VERSION>.snap --devmode --dangerous

nomacs
```

## Resources

Here is a YAML validator: http://nodeca.github.io/js-yaml/

## Missing features

- ~~Quazip~~
- ~~themes~~
- Plugins

## Issues

as of 05.09.18 17:00

```bash
Gtk-Message: Failed to load module "overlay-scrollbar"
Gtk-Message: Failed to load module "unity-gtk-module"
Gtk-Message: Failed to load module "canberra-gtk-module"
Qt: Session management error: None of the authentication protocols specified are supported
```