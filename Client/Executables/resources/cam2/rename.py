import os
count = 1858

for i in range(len(os.listdir("."))):
        dst ="frame" + str(count-601 +i) +".jpg"
        src ='frame'+ str(count + i) + ".jpg"
        # rename() function will
        # rename all the files
        os.rename(src, dst)
