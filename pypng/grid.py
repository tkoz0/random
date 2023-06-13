import png

sizex = 10
sizey = 4

cellsize = 24
linesize = 2

imgrows = sizey*cellsize + linesize
imgcols = sizex*cellsize + linesize

BACK = [255,255,255]
LINE = [0,0,0]

img = [BACK*imgcols for _ in range(imgrows)]

for r in range(imgrows):
    for c in range(imgcols):
        if r % cellsize < linesize or c % cellsize < linesize:
            img[r][3*c:3*c+3] = LINE

COLORS = [[255,0,0],[0,255,0],[0,0,255],[255,255,0]]
start_x = 0
for s in range(1,5):
    for r in range(s*cellsize):
        for c in range(start_x*cellsize,(start_x+s)*cellsize):
            if r % cellsize >= linesize and c % cellsize >= linesize:
                img[r][3*c:3*c+3] = COLORS[s-1]
    start_x += s

png.from_array(img,'RGB').save('test.png')
