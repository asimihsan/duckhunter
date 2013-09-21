class gyp {
    Class['development_tools'] -> Class['gyp']
    exec { 'svn checkout gyp':
        command => 'svn checkout http://gyp.googlecode.com/svn/trunk/ gyp',
        cwd => '/usr/local/',
        creates => '/usr/local/gyp/.svn',
    }
}