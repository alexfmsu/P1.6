 - 3 groups
 - 1 ssh key per group, to be added by hand to known_hosts on each name  

To setup the cluster
===================

add the group public key to known_hosts

# System-wide
sudo apt-get install git
sudo apt-get install python-pip
sudo apt-get install libhdf5-serial-1.8.4  libhdf5-serial-dev
sudo apt-get install netcdf-bin libnetcdf6 libnetcdf-dev
sudo apt-get install python-dev

# Use already existent one
cd /home/ubuntu
tar -zxvf venv.tar.gz


# if unpacking fails, to build virtualenv use 
# Create new virtualev
sudo pip install virtualenv
virtualenv /home/ubuntu/venv
virtualenv --relocatable /home/ubuntu/venv
source /home/ubuntu/venv/bin/activate
pip install numpy
pip install netCDF
 
For the users
=============

users needs to modify their /etc/hosts


g1  
  - Jimmy Aguilar Mena 
  - Mauro Bardelloni
  - Moreno Baricevic
  - Hemen Hosseini
  - Eis Annavini

g2
  - Marco Borelli
  - Nicola Giulianio
  - Eric Pascolo
  - Giuseppe Puglisi
  - Edwin Fernando Posada Correa

g4 
  - Stefano Piani
  - Marco Raveri
  - Leonardo Romor
  - Marco Tezzele 
