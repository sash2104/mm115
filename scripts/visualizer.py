""" """

import argparse
import sys
import copy
import tkinter as tk
import tkinter.messagebox as tkm
from PIL import Image

def parse_arguments():
    parser = argparse.ArgumentParser(description=' ')
    parser.add_argument('-i', '--input_file', required=True)
    parser.add_argument('-o', '--output_file', required=True)
    parser.add_argument('-a', '--answer_file', required=True)
    parser.add_argument('-p', '--image_file', required=True)
    args = parser.parse_args()
    return args

W = 600
H = 600

def draw_ans(canvas, mat0, edges):
    dx = 0
    # indexのための番兵を足す
    mat = [[0]+list(range(len(mat0)))]
    for i, l in enumerate(mat0):
        mat.append([i]+l)
    n = len(mat)
    size = W//n
    given = set()
    for edge in edges:
        given.add((edge[0]+1, edge[1]+1))
        given.add((edge[1]+1, edge[0]+1))
    for i in range(n):
        for j in range(n):
            color = None
            if (i,j) in given:
                color = 'green'
            if i == 0 or j == 0:
                color = 'gray'
            canvas.create_rectangle(dx+i*size, j*size, dx+(i+1)*size, (j+1)*size, fill = color, outline ='#00f')         # 四角形で塗りつぶし
            if mat[i][j] != INF:
                canvas.create_text(dx+i*size+size//2, j*size+size//2, text=str(mat[i][j]), font=("Helvetica", size//2))

def draw_out(canvas, out, ans, edges):
    dx = 700
    # indexのための番兵を足す
    mat = [[0]+list(range(len(out)))]
    for i, l in enumerate(out):
        mat.append([i]+l)
    n = len(mat)
    size = W//n
    given = set()
    for edge in edges:
        given.add((edge[0]+1, edge[1]+1))
        given.add((edge[1]+1, edge[0]+1))
    for i in range(n):
        for j in range(n):
            color = None
            if i == 0 or j == 0:
                color = 'gray'
            elif out[i-1][j-1] == ans[i-1][j-1]:
                color = 'green'
            elif out[i-1][j-1] < ans[i-1][j-1]:
                if (i,j) in given:
                    color = 'red'
                else:
                    color = 'orange'
            canvas.create_rectangle(dx+i*size, j*size, dx+(i+1)*size, (j+1)*size, fill = color, outline ='#00f')         # 四角形で塗りつぶし
            if mat[i][j] != INF:
                canvas.create_text(dx+i*size+size//2, j*size+size//2, text=str(mat[i][j]), font=("Helvetica", size//2))

def load_input(file):
    with open(file) as f:
        n = int(f.readline())
        c = float(f.readline())
        k = int(f.readline())
        e = int(f.readline())
        # print(n, c, k, e)
        edges = []
        for i in range(e):
            fr, to, co = list(map(int, f.readline().strip().split()))
            edges.append((fr, to, co))
    return n, edges

INF = 999
def warshall_floyd(mat):
    n = len(mat)
    dist = copy.deepcopy(mat)
    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            if dist[i][j] == 0:
                dist[i][j] = INF
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if dist[i][k] == INF or dist[k][j] == INF:
                    continue
                d = dist[i][k] + dist[k][j]
                if dist[i][j] > d:
                    dist[i][j] = d
    return dist

def load_output(file):
    with open(file) as f:
        n = int(f.readline())
        ret = []
        for i in range(n):
            s = f.readline()
            ret.append(list(map(int,list(s.strip()))))
    return ret

def matshow(mat):
    for l in mat:
        print(l)
    print()

def main(args):
    n, edges = load_input(args.input_file)
    if n > 40:
        # 表示してもどうせ見えないので何もしない
        return
    mat_out = load_output(args.output_file)
    dist_out = warshall_floyd(mat_out)
    # matshow(mat_out)
    # matshow(dist_out)
    mat_ans = load_output(args.answer_file)
    dist_ans = warshall_floyd(mat_ans)
    # matshow(mat_ans)
    # matshow(dist_ans)

    # draw
    root = tk.Tk()
    root.title(u'marathon match 115')
    root.geometry('1600x1200')

    canvas = tk.Canvas(root, width = 1300, height = 600)
    canvas.pack()

    draw_ans(canvas, dist_ans, edges)
    draw_out(canvas, dist_out, dist_ans, edges)

    canvas.place(x=0, y=0)
    # Tkinter canvas object can only be saved as a postscipt file
    # which is actually a postscript printer language text file
    canvas.update()
    canvas.postscript(file="tmp.matrix.eps",colormode='color')
    save_as_png(args.image_file)

    # guiで確認したかったらコメントアウト外す
    # root.mainloop()


def save_as_png(filename):
    # save postscipt image 
    img = Image.open('tmp.matrix.eps') 
    img.save(filename, 'png') 


if __name__ == '__main__':
    args = parse_arguments()
    main(args)

