class development_tools {
    case $operatingsystem {
        Fedora: {
            exec { 'yum --assumeyes groupinstall "Development Tools"':
                unless => 'yum grouplist "Development Tools" | grep "^Installed Groups"',
            }
        }
    }
    $base_packages = [
        "git",
        "htop",
        "subversion",
        "wget",
    ]
    case $operatingsystem {
        ubuntu: {
            $packages = ["build-essential"]
        }
        Fedora: {
            $packages = ["automake",
                         "gcc",
                         "gcc-c++",                
                         "make"]
        }
    }
    package { $base_packages: ensure => installed }
    package { $packages: ensure => installed }
}
