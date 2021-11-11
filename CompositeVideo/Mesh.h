#pragma once
#include "Matrix.h"

template<class Graphics>
class Mesh
{
  public:
  int vertexCount;
  const float (*vertices)[3];
  int edgeCount;
  const unsigned short (*edges)[2];
  int triangleCount;
  const unsigned short (*triangles)[3];
  const float (*triangleNormals)[3];
  short (*tvertices)[3];
  signed char (*tTriNormals)[3];

  Mesh(int vertCount, const float verts[][3], int edgeCount_ = 0, const unsigned short edges_[][2] = 0, int triCount = 0, const unsigned short tris[][3] = 0, const float triNorms[][3] = 0)
    :vertexCount(vertCount),
    vertices(verts),
    edgeCount(edgeCount_),
    edges(edges_),
    triangleCount(triCount),
    triangles(tris),
    triangleNormals(triNorms)
  {
    tvertices = (short(*)[3]) malloc(sizeof(short) * 3 * vertexCount);
    if(triangleNormals)
      tTriNormals = (signed char(*)[3]) malloc(sizeof(signed char) * 3 * triangleCount);
  }

  ~Mesh()
  {
    delete(tvertices);
  }

  void drawTriangles(Graphics &g, char color)
  {
    const float scaleN = 1.0f / 127.0f;
    
    for(int i = 0; i < triangleCount; i++)
    {
      short *v0 = tvertices[triangles[i][0]];
      short *v1 = tvertices[triangles[i][1]];
      short *v2 = tvertices[triangles[i][2]];
      int dx1 = v1[0] - v0[0];
      int dy1 = v1[1] - v0[1]; 
      int dx2 = v2[0] - v0[0];
      int dy2 = v2[1] - v0[1];
      if(dx1 * dy2 - dx2 * dy1 < 0)
      {
        char c;
        if(tTriNormals)
        {
          // with L = { 0, 0, -1 }
          //float nz = tTriNormals[i][2] * scaleN;
          //float NdotL = max(0.0f, -nz);

          const signed char *normal = tTriNormals[i];

          const float nx = normal[0] * scaleN;
          const float ny = normal[1] * scaleN;
          const float nz = normal[2] * scaleN;

          const float L[3] = { 0, 0, -1 }; 
          
          const float NdotL = max(0.0f, nx * L[0] + ny * L[1] + nz * L[2]);
          c = (char) (color * NdotL + 0.5);
        }
        else
          c = i % 40;
        g.enqueueTriangle(tvertices[triangles[i][0]], tvertices[triangles[i][1]], tvertices[triangles[i][2]], c);
      }
    }
  }

  void drawEdges(Graphics &g, char color)
  {
    for(int i = 0; i < edgeCount; i++)
    {
      g.line(tvertices[edges[i][0]][0], tvertices[edges[i][0]][1], tvertices[edges[i][1]][0], tvertices[edges[i][1]][1], color);
    }
  }
    
  void drawVertices(Graphics &g, char color)
  {
    for(int i = 0; i < vertexCount; i++)
      g.dot(tvertices[i][0], tvertices[i][1], color);
  }

  void transform(Matrix m, Matrix normTrans = Matrix())
  {
    for(int i = 0; i < vertexCount; i++)
    {
      Vector v = m * Vector(vertices[i][0], vertices[i][1], vertices[i][2]);
      tvertices[i][0] = v[0] / v[3];
      tvertices[i][1] = v[1] / v[3];
      tvertices[i][2] = v[2];
    }
    if(triangleNormals)
      for(int i = 0; i < triangleCount; i++)
      {
        Vector v = normTrans * Vector(triangleNormals[i][0], triangleNormals[i][1], triangleNormals[i][2]);
        tTriNormals[i][0] = v[0] * 127;
        tTriNormals[i][1] = v[1] * 127;
        tTriNormals[i][2] = v[2] * 127;
      }
  }
};

