Exec {
    path => "/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin",
    logoutput => "on_failure",
}

File {
    owner => "vagrant",
    group => "vagrant",
}

include development_tools
include environment
include gyp
