#include "MDLReader.h"
#include "Vertex.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

MDLReader::MDLReader()
{
}

bool MDLReader::LoadMDL(const char* pPath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
   m_BasePath = string(pPath);
   return ReadVVD(vertices) && ReadVTX(indices);
}

bool MDLReader::ReadVTX(std::vector<uint32_t>& indices) {
    streampos size;
    bool success = false;

    string vtxPath = m_BasePath + ".sw.vtx";
    ifstream file(vtxPath, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        unsigned char *vtxData = new unsigned char[size];
        cout << "Size: " << size << endl;
        file.seekg(0, ios::beg);
        file.read((char*) vtxData, size);

        VTX::FileHeader_t *header = (VTX::FileHeader_t*)vtxData;

        cout << "Read the VTX file!" << endl;
        PrintHeader(*header);

        for (int i = 0; i < header->numBodyParts; i++)
        {
            VTX::BodyPartHeader_t *bodyPart = header->pBodyPart(i);
            for (int j = 0; j < bodyPart->numModels; j++)
            {
                VTX::ModelHeader_t *model = bodyPart->pModel(j);
                for (int k = 0; k < model->numLODs; k++)
                {
                    VTX::ModelLODHeader_t *modelLOD = model->pLOD(k);
                    for (int l = 0; l < modelLOD->numMeshes; l++)
                    {
                        VTX::MeshHeader_t *mesh = modelLOD->pMesh(l);
                        for (int m = 0; m < mesh->numStripGroups; m++)
                        {
                            VTX::StripGroupHeader_t *stripGroup = mesh->pStripGroup(m);

                            unsigned short *pIndices = new unsigned short[stripGroup->numIndices];
                            memcpy(pIndices, stripGroup->pIndex(0), stripGroup->numIndices * sizeof(unsigned short));

                            // Build the mapping from strip group vertex idx to actual mesh idx
                            unsigned short *pGroupIndexToMeshIndex = new unsigned short[stripGroup->numVerts];

                            for (int stripVertIndx = 0; stripVertIndx < stripGroup->numVerts; ++stripVertIndx)
                            {
                                pGroupIndexToMeshIndex[stripVertIndx] = stripGroup->pVertex(stripVertIndx)->origMeshVertID;
                            }

                            for (int n = 0; n < stripGroup->numStrips; n++)
                            {
                                VTX::StripHeader_t *strp = stripGroup->pStrip(n);
                                if (strp->flags & VTX::StripHeaderFlags_t::STRIP_IS_TRILIST)
                                {
                                    for (int strpIndx = 0; strpIndx < strp->numIndices; strpIndx += 3)
                                    {
                                        int idx = strp->indexOffset + strpIndx;
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx]]);
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx + 1]]);
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx + 2]]);
                                    }
                                }
                                else
                                {
                                    assert(strp->flags & VTX::StripHeaderFlags_t::STRIP_IS_TRISTRIP);
                                    for (int strpIndx = 0; strpIndx < strp->numIndices - 2; ++strpIndx)
                                    {
                                        int idx = strp->indexOffset + strpIndx;
                                        bool ccw = (strpIndx & 0x1) == 0;
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx]]);
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx + 1 + ccw]]);
                                        indices.push_back(pGroupIndexToMeshIndex[pIndices[idx + 2 - ccw]]);
                                    }
                                }
                            }

                            delete[] pGroupIndexToMeshIndex;
                            delete[] pIndices;
                        }
                    }
                }
            }
        }

        success = true;

        delete[] vtxData;

        file.close();
    }
    else
        cout << "Unable to open file." << endl;

    return success;
}

bool MDLReader::ReadVVD(vector<Vertex> &vertices) {
    streampos size;
    bool success = false;

    string vvdPath = m_BasePath + ".vvd";
    ifstream file(vvdPath, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        cout << "Size: " << size << endl;
        file.seekg(0, ios::beg);
        VVD::vertexFileHeader_t header;
        file.read((char*)&header, sizeof(VVD::vertexFileHeader_t));

        cout << "Read the VVD file!" << endl;
        PrintHeader(header);

        if (header.numFixups == 0)
        {
            // Offset to the start of vertices
            size_t vertexChunkSize = header.tangentDataStart - header.vertexDataStart;

            // Read all the vertices
            file.seekg(header.vertexDataStart);
            for (size_t i = 0; i < vertexChunkSize; i += 48)
            {
                VVD::mstudiovertex_t valvVertex;
                file.read((char*)&valvVertex, sizeof(VVD::mstudiovertex_t));
                
                Vertex vertex = {};

                vertex.pos = {
                    valvVertex.m_posX,
                    valvVertex.m_posY * -1.0f,
                    valvVertex.m_posZ,
                };

                vertex.texCoord = {
                    valvVertex.m_texCoordU,
                    valvVertex.m_texCoordV
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                vertices.push_back(vertex);
            }

            success = true;
        }
        else
        {
            // TODO: handle having fixups
        }

        file.close();
    }
    else
        cout << "Unable to open file." << endl;

    return success;
}

void MDLReader::PrintVertex(VVD::mstudiovertex_t& vert)
{
    cout << "Pos: (" << vert.m_posX << ", " << vert.m_posY << ", " << vert.m_posZ << ")" << endl;
    cout << "Tex: (" << vert.m_texCoordU << ", " << vert.m_texCoordV << ")" << endl;
}

void MDLReader::PrintHeader(VVD::vertexFileHeader_t& header)
{
    cout << "ID: " << header.id << endl;
    cout << "Version: " << header.version << endl;
    cout << "Checksum: " << header.checksum << endl;
    cout << "numLODs: " << header.numLODs << endl;
    cout << "numFixups: " << header.numFixups << endl;
    cout << "fixupTableStart: " << header.fixupTableStart << endl;
    cout << "vertexDataStart: " << header.vertexDataStart << endl;
    cout << "tangentDataStart: " << header.tangentDataStart << endl;
    for (int i = 0; i < MAX_NUM_LODS; i++)
    {
        cout << "\t numLODVertexes[" << i << "]: " << header.numLODVertexes[i] << endl;
    }
}

void MDLReader::PrintHeader(VTX::FileHeader_t& header)
{
    // file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
    cout << "Version: " << header.version << endl;

    // hardware params that affect how the model is to be optimized.
    cout << "VertCacheSize: " << header.vertCacheSize << endl;
    cout << "MaxBonesPerStrip: " << header.maxBonesPerStrip << endl;
    cout << "MaxBonesPerTri: " << header.maxBonesPerTri << endl;
    cout << "MaxBonesPerVert: " << header.maxBonesPerVert << endl;

    // must match checkSum in the .mdl
    cout << "Checksum: " << header.checkSum << endl;

    // Also specified in ModelHeader_t's and should match
    cout << "Num LODs: " << header.numLODs << endl;

    // Offset to materialReplacementList Array. one of these for each LOD, 8 in total
    cout << "MaterialReplacementList Offset: " << header.materialReplacementListOffset << endl;

    //Defines the size and location of the body part array
    cout << "Num BodyParts: " << header.numBodyParts << endl;
    cout << "Bodyparts Offset: " << header.bodyPartOffset << endl;
}
