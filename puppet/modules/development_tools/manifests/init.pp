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
        "valgrind",
        "wget",
    ]
    case $operatingsystem {
        ubuntu: {
            $packages = ["build-essential",
                         "libffi",
                         "libffi-dev",
                         "zlib",
                         "zlib1g-dev"]
        }
        Fedora: {
            $packages = ["automake",
                         "gcc",
                         "gcc-c++", 
                         "libffi",
                         "libffi-devel",              
                         "make",
                         "zlib",
                         "zlib-devel",
                         "zlib-static"]
        }
    }
    package { $base_packages: ensure => installed }
    package { $packages: ensure => installed }
}
