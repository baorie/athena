#!/usr/bin/python

#Rielle Quiambao
#August 4, 2015
#ppmvisual.py 

import os, errno, shutil
from PIL import Image
from pylab import *

#YOU WILL NEED: Pillow (https://python-pillow.github.io/)
#				ffmpeg (http://www.ffmpeg.org/)

#REMEMBER TO RUN 'athinput.kh' BEFORE RUNNING THIS PYTHON SCRIPT

#base directory variable
bdir = '/Users/riellequiambao/kh/run/test'

	
#------------------------------------------
#function convertimg: reads .ppm files, truncates initial file extension and turns them into .jpg files
#def toJPG(filename):
#	if not '.jpg' in filename:
#		temp = filename.split('ppm')[0]
#		im = Image.open(filename)
#		im.save(temp+'jpg')
#		os.remove(bdir+'/jpgs/'+filename)


#------------------------------------------
#function converting: reads .ppm files, truncates initial file extension and turns them into .png files
def toPNG(filename):
	if '.ppm' in filename:
		temp = filename.split('ppm')[0]
		im = Image.open(filename)
		im.save(temp+'png')
		os.remove(bdir+'/pngs/'+filename)
    
#------------------------------------------
#function separatefiles(): separates files according to filetypes
# if ppm file, copies them into a new jpg folder; if bin, copies into binfiles folder
# checks for ppm files in ppmfile folder and transfers them into jpg folder
def separatefiles():
	ans = None
	validans = False
	while validans == False :
		lookother = raw_input('Are your .ppm files and .bin files in the same directory as athinput.kh (i.e. Your files are NOT situated in folders that specify their file extensions)? Please enter 1 for yes or 0 for no: ')
		if lookother.isdigit() and (lookother == '0' or lookother == '1'):
			ans = int(lookother)
			validans= True
		else: 
			'Please enter a valid answer.'
	
	if ans == 1 :
		for filename in os.listdir(bdir):
			if '.ppm' in filename: 
				shutil.copyfile(bdir+'/'+filename, bdir+'/pngs/'+filename)
				shutil.move(bdir+'/'+filename, bdir+'/ppmfiles')
			if '.bin' in filename:
				shutil.move(bdir+'/'+filename, bdir+'/binfiles')
	elif ans == 0:
		os.chdir(bdir+'/ppmfiles')
		for filename in os.listdir(bdir+'/ppmfiles'):
			if '.ppm' in filename:
				shutil.copyfile(bdir+'/ppmfiles/'+filename, bdir+'/pngs/'+filename)
	
#------------------------------------------
#function mknewdir: creates new directories
def mknewdir(path):
	try:
		os.makedirs(path)
	except OSError as exc:
		if exc.errno == errno.EEXIST and os.path.isdir(path):
			pass
		else:
			raise
			
#------------------------------------------
#function PNGtoMP4(): converts png  files into frames that make up an .mp4 video file
#requires user input for fps and saves according to fps
#checks if input is a number 
#moves new videos into mp4 folder
def PNGtoMP4():
	numset = False
	while numset == False:
		fps = raw_input('Enter framerate: ')
		if fps.isdigit():
			numset = True
			temp = int(fps)
			filename = raw_input('What would you like to name this video? ')+'.mp4'
			os.system('ffmpeg -framerate ' + fps + ' -pattern_type glob -i \'*.png\' -c:v libx264 '+ filename)
			shutil.move(bdir+'/pngs/'+filename, bdir+'/mp4s/'+filename)
			#os.system('mv '+ filename +' '+ bdir+'/mp4s')
		elif fps.isdigit() == False:
			print('Please enter a valid number')

#------------------------------------------
#function PPMtoMP4(): converts ppm files into frames that make up an .mp4 video file
#requires user input for fps and saves according to fps
#checks if input is a number 
#moves new videos into mp4 folder			

def PPMtoMP4():
	numset = False
	while numset == False:
		fps = raw_input('Enter framerate: ')
		if fps.isdigit():
			numset = True
			temp = int(fps)
			filename = raw_input('What would you like to name this video? ')+'.mp4'
			#os.system('ffmpeg -framerate ' + fps + ' -pattern_type glob -i \'*.jpg\' -c:v libx264 '+ filename)
			os.system('ffmpeg -framerate ' + fps + ' -pattern_type glob -i \'*.ppm\' -c:v libx264 '+ filename)
			shutil.move(bdir+'/ppmfiles/'+filename, bdir+'/mp4s/'+filename)
			#os.system('mv '+ filename +' '+ bdir+'/mp4s')
		elif fps.isdigit() == False:
			print('Please enter a valid number')

#------------------------------------------
#function makevideo(): helps user decide what file format they want to convert to and calls toMP4 methods to create video

def makevideo():
	ans = None
	validans = False
	while validans == False :
		videotype = raw_input('What image type are you converting from? Please type in either \'ppm\' or \'png\'? ')
		if videotype == 'ppm' or videotype == 'png':
			ans = videotype
			validans= True
		else: 
			'Please enter a valid answer.'
	
	if ans == 'ppm':
		os.chdir(bdir+'/ppmfiles')
		PPMtoMP4()
	elif ans == 'png':
		os.chdir(bdir+'/pngs')
		for filename in os.listdir(bdir+'/pngs'):
			toPNG(filename)
		PNGtoMP4()
	
	
	
#------------------------------------------
#main method
def main():
	#print(os.getcwd())
	os.chdir('../test')
	#print(os.getcwd())
	#mknewdir(bdir+'/jpgs')
	mknewdir(bdir+'/pngs')
	mknewdir(bdir+'/mp4s')
	mknewdir(bdir+'/binfiles')
	mknewdir(bdir+'/ppmfiles')
	separatefiles()	
	makevideo()

if  __name__ =='__main__':
    main()
