# duckhunter

Monitor what SSH users type and execute on a server

## Installation

### Building from source in a VM

Requirements:

-   Git
-   Vagrant
-   VirtualBox

```
git clone git@github.com:asimihsan/duckhunter.git
cd duckhunter
git submodule init
git submodule update

vagrant up fedora-18-x86_64
vagrant ssh fedora-18-x86_64
cd /vagrant
make deps
make duckhunter

./build/makefiles/out/Default/duckhunter
```