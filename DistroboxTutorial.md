# distrobox_tutorial
dyros_tocabi install tutorial for ubuntu 24.04

install distrobox
```sh
~$ sudo apt install curl
~$ curl https://raw.githubusercontent.com/89luca89/distrobox/main/install | sudo sh
~$ sudo apt install podman
```

create ubuntu-20-04 container with distrobox
```sh
~$ mkdir ubuntu-20-04  # create home directory for ubuntu 20.04 distrobox
~$ distrobox create --image ubuntu:20.04 --name ubuntu-20-04 --hostname ubuntu-20-04 --home ~/ubuntu-20-04 --nvidia
~$ distrobox enter ubuntu-20-04
```

install ros in ubuntu-20-4 container
```sh
~$ sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu focal main" > /etc/apt/sources.list.d/ros-latest.list'
~$ curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
~$ sudo apt update
~$ sudo apt install ros-noetic-desktop-full
~$ echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
~$ source ~/.bashrc
```
