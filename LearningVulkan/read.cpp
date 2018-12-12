#include <stdio.h>
#include <iostream>
#include <fstream>

class MDLReader {

public:
    void ReadVTX();
    void ReadVVD();
};

using namespace std;

#define MAX_NUM_LODS 8
#define MAX_NUM_BONES_PER_VERT 3

namespace VVD
{
    // these structures can be found in <mod folder>/src/public/studio.h
    struct vertexFileHeader_t
    {
        int	id;				// MODEL_VERTEX_FILE_ID
        int	version;			// MODEL_VERTEX_FILE_VERSION
        long	checksum;			// same as studiohdr_t, ensures sync
        int	numLODs;			// num of valid lods
        int	numLODVertexes[MAX_NUM_LODS];	// num verts for desired root lod
        int	numFixups;			// num of vertexFileFixup_t
        int	fixupTableStart;		// offset from base to fixup table
        int	vertexDataStart;		// offset from base to vertex block
        int	tangentDataStart;		// offset from base to tangent block
    };

    // apply sequentially to lod sorted vertex and tangent pools to re-establish mesh order
    struct vertexFileFixup_t
    {
        int	lod;			// used to skip culled root lod
        int	sourceVertexID;		// absolute index from start of vertex/tangent blocks
        int	numVertexes;
    };


    // NOTE: This is exactly 48 bytes
    struct mstudiovertex_t
    {
        mstudioboneweight_t	m_BoneWeights;
        float m_posX;
        float m_posY;
        float m_posZ;
        float m_normalX;
        float m_normalY;
        float m_normalZ;
        float m_texCoordU;
        float m_texCoordV;
    };

    // 16 bytes
    struct mstudioboneweight_t
    {
        float	weight[MAX_NUM_BONES_PER_VERT];
        char	bone[MAX_NUM_BONES_PER_VERT]; 
        unsigned char	numbones;
    };
}


void MDLReader::ReadVTX() {

}

void MDLReader::ReadVVD() {
    streampos size;
    char *memblock;

    ifstream file("models/mdl/", ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        cout << "the entire file content is in memory";

        delete[] memblock;
    }
    else
        cout << "Unable to open file." << endl;
    
}

int main(int argc, char const *argv[])
{
    MDLReader r;
    r.ReadVVD();
    return 0;
}
