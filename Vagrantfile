Vagrant.configure("2") do |config|
    config.vm.provision :puppet do |puppet|
      puppet.module_path = "puppet/modules"
      puppet.manifests_path = "puppet"
      puppet.manifest_file  = "init.pp"    
      puppet.options = "--verbose --debug"
    end

    config.vm.define "fedora-18-x86_64" do |master|
        master.vm.box = "fedora-18-x86_64"
        master.vm.box_url = "http://puppet-vagrant-boxes.puppetlabs.com/fedora-18-x64-vbox4210.box"
    end

end
