MANDELBULBER 0.91

autor: Krzysztof Marczak
contact: buddhi1980@gmail.com
www: http://sourceforge.net/projects/mandelbulber/

LICENCE: GNU GPL v3.0

1. REQUIREMENTS

- Linux OS with installed GTK+2.0 libraries, libjpeg, libsndfile1
- very fast CPU (recomended 2GHz, 4 cores)
- 500MB RAM


2. INSTALATION

Please type:
sudo ./install

Program will be automaticaly copied to /usr/bin directory and additinal data to ~/.mandelbulber directory

Program was compiled for x86 CPUs with SSE


3. COMPILATION

Program is prepared for compilation using gcc. Before compilation you need to install libgtk2.0-dev and libsndfile1-dev package

sudo apt-get install libgtk2.0-dev
sudo apt-get install libsndfile1-dev

next go to src/Release directory and type:
make clean
make all

program was compiled for your native CPU. If you have x64 CPU now will be able to render very high resolution images.

go back to mandelbulber directory and type:
sudo ./install

Source files was created in Enclipse SDK CDT

4. TESTED ON:

- Ubuntu 9.10 x64
- Ubuntu 9.10 x86
- Debian Squeese

5. USAGE

Please download user manual: http://sourceforge.net/projects/mandelbulber/files/Mandelbulber%20user%20guide.pdf/download

command-line mode:

Syntax:
mandelbulber [options...] [settings_file]
options:
  -nogui          - start program without GUI
  -flight         - render flight animation
  -keyframe       - render keyframe animation
  -start N        - start renderig from frame number N
  -end            - rendering will end on frame number N
  -format FORMAT  - image output format
     jpg - JPEG format
     png - PNG format
     png16 - 16-bit PNG format
     png16alpha - 16-bit PNG with alpha channel format
[settings_file] - file with fractal settings (program also tries
to find file in ./mandelbulber/settings directory)
When settings_file is put as command argument then program will start in noGUI mode

