class environment {
    file { "/home/vagrant/.bashrc":
        source => "/vagrant/puppet/modules/environment/files/bashrc"
    }
}
