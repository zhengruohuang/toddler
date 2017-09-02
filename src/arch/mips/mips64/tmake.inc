#!/usr/bin/python


global_tools = { \
    '.c' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-gcc',
        'flags' : '-c -g -nostdlib -fno-builtin -nostartfiles -fno-stack-protector -mno-abicalls -fno-pic -march=mips64r2 -mabi=64 -O2',
        'inc'   : '-I__inc__',
        'direct': '__src__',
        'obj'   : '-o __obj__ __src__',
        'dep'   : '-MM __src__ > __dep__',
    },

    '.S' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-gcc',
        'flags' : '-c -nostdlib -fno-builtin -nostartfiles -fno-stack-protector -mno-abicalls -fno-pic -march=mips64r2 -mabi=64 -O0',
        'inc'   : '-I__inc__',
        'direct': '__src__',
        'obj'   : '-o __obj__ __src__',
        'dep'   : '-M __src__ > __dep__',
    },
    
    'ld' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-ld',
        'flags' : '-s --no-undefined',
        'script': '-T __script__',
        'obj'   : '-o __target__ __obj__',
    },
    
    'ar' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-ar',
        'flags' : 'crD',
        'obj'   : '__target__ __obj__',
    },
    
    'strip' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-strip',
        'flags' : '',
        'obj'   : '__target__',
    },
    
    'gen_bin' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-objcopy',
        'flags' : '-O binary',
        'obj'   : '__src__ __target__',
    },
}

boot_tools = { \
    '.c' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-gcc',
        'flags' : '-c -nostdlib -fno-builtin -nostartfiles -fno-stack-protector -mno-abicalls -fno-pic -march=mips64r2 -mabi=64 -O2',
        'inc'   : '-I__inc__',
        'direct': '__src__',
        'obj'   : '-o __obj__ __src__',
        'dep'   : '-MM __src__ > __dep__',
    },

    '.S' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-gcc',
        'flags' : '-c -nostdlib -fno-builtin -nostartfiles -fno-stack-protector -mno-abicalls -fno-pic -march=mips64r2 -mabi=64 -O0',
        'inc'   : '-I__inc__',
        'direct': '__src__',
        'obj'   : '-o __obj__ __src__',
        'dep'   : '-M __src__ > __dep__',
    },
    
    'ld' : { \
        'exec'  : mips_endian_prefix + '-linux-gnuabi64-ld',
        'flags' : '-s --no-undefined',
        'script': '-T __script__',
        'obj'   : '-o __target__ __obj__',
    },
}

def build_loader():
    print_info('loader', 'Building loader')
    
    loader_files = []
    for d in arch_dirs:
        loader_files += get_all_files(d + 'loader/', [ '.S', '.c' ])
    
    loader_ld = arch_dir_map['mips64'] + 'loader/loader.ld'
    
    build_files(
        files = loader_files,
        target_name = bin_dir + 'tdlrldr.elf',
        ext_dep = [ loader_ld ],
        ext_flags = { 'ld/script' : loader_ld },
        tools = arch_tools['boot_tools']
    )
    
def build_hal():
    print_info('hal', 'Building HAL')
    
    hal_files = get_all_files(src_dir + 'hal/', [ '.c' ])
    for d in arch_dirs:
        hal_files += get_all_files(d + 'hal/', [ '.c', '.asm', '.s', '.S' ])
    
    hal_ld = arch_dir_map['mips64'] + 'hal/hal.ld'
    
    build_files(
        hal_files,
        bin_dir + 'tdlrhal.bin',
        ext_dep = [ hal_ld ],
        ext_flags = { 'ld/script' : hal_ld },
    )
    
def build_arch():
    # Build loader
    if 'build_loader' in arch_funcs:
        arch_funcs['build_loader']()
    
    # Build HAL
    if 'build_hal' in arch_funcs:
        arch_funcs['build_hal']()
    
def build_disk():
    print_info('floppyimg', 'Building floppy image')

def qemu8():
    print_info('emulator', 'Starting SMP QEMU')
    
    loader_name = bin_dir + 'tdlrldr.elf'
    coreimg_name = loader_name
    
    # Compose the cmd
    cmd = 'qemu-system-' + mips_endian_prefix + \
        ' -cpu MIPS64R2-generic -M malta -m 128' + \
        ' -nographic -serial mon:stdio' + \
        ' -smp cores=2,threads=2,sockets=2' + \
        ' -kernel ' + loader_name + \
        ' -initrd ' + coreimg_name
    
    # Execute the cmd
    code = exec_cmd(cmd)
    assert(code == 0)
    
def qemu():
    print_info('emulator', 'Starting QEMU')
    
    loader_name = bin_dir + 'tdlrldr.elf'
    coreimg_name = bin_dir + 'tdlrcore.img'
    
    # Compose the cmd
    cmd = 'qemu-system-' + mips_endian_prefix + \
        ' -cpu MIPS64R2-generic -M malta -m 128' + \
        ' -nographic -serial mon:stdio' + \
        ' -kernel ' + loader_name + \
        ' -initrd ' + coreimg_name
    
    # 5KEf
    
    # Execute the cmd
    code = exec_cmd(cmd)
    assert(code == 0)
    
def start_emu():
    emu_type = 'qemu'
    if emu_type in arch_funcs:
        arch_funcs[emu_type]()
    

# Setup global tools
arch_tools['global_tools'] = global_tools

# Setup arch tools
arch_tools['boot_tools'] = boot_tools

# Setup callback functions
arch_funcs['build_arch'] = build_arch
arch_funcs['build_disk'] = build_disk

arch_funcs['build_loader'] = build_loader
arch_funcs['build_hal'] = build_hal

arch_funcs['qemu'] = qemu
arch_funcs['qemu8'] = qemu8
arch_funcs['start_emu'] = start_emu
