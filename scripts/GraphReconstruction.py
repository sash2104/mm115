import sys

class GraphReconstruction:
  def findSolution(self, N, C, K, paths):
      
    out = []

    for i in range(N):
      row = ""
      for k in range(N): row += "1"
      out.append(row)

    return out


N = int(input())
C = float(input())
K = int(input())
NumPaths = int(input())
paths = []
for i in range(NumPaths): paths.append(input())
  
prog = GraphReconstruction()
ret = prog.findSolution(N, C, K, paths)
print(len(ret))
for st in ret:
  print(st)
sys.stdout.flush()