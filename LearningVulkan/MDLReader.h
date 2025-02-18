#pragma once

#include <vector>
#include <tiny_obj_loader.h>

#define MAX_NUM_LODS 8
#define MAX_NUM_BONES_PER_VERT 3

typedef unsigned char byte;

namespace VVD
{
#pragma pack(1)
    // these structures can be found in <mod folder>/src/public/studio.h
    struct vertexFileHeader_t
    {
        int	id;				// MODEL_VERTEX_FILE_ID
        int	version;			// MODEL_VERTEX_FILE_VERSION
        int checksum;			// same as studiohdr_t, ensures sync
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

    // 16 bytes
    struct mstudioboneweight_t
    {
        float	weight[MAX_NUM_BONES_PER_VERT];
        char	bone[MAX_NUM_BONES_PER_VERT];
        unsigned char	numbones;
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
#pragma pack()
}

// Taken from optimize.h
namespace VTX
{
#pragma pack(1)
    struct BoneStateChangeHeader_t
    {
        int hardwareID;
        int newBoneID;
    };

    struct Vertex_t
    {
        // these index into the mesh's vert[origMeshVertID]'s bones
        unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
        unsigned char numBones;

        unsigned short origMeshVertID;

        // for sw skinned verts, these are indices into the global list of bones
        // for hw skinned verts, these are hardware bone indices
        char boneID[MAX_NUM_BONES_PER_VERT];
    };

    enum StripHeaderFlags_t
    {
        STRIP_IS_TRILIST = 0x01,
        STRIP_IS_TRISTRIP = 0x02
    };

    // a strip is a piece of a stripgroup that is divided by bones 
    // (and potentially tristrips if we remove some degenerates.)
    struct StripHeader_t
    {
        // indexOffset offsets into the mesh's index array.
        int numIndices;
        int indexOffset;

        // vertexOffset offsets into the mesh's vert array.
        int numVerts;
        int vertOffset;

        // use this to enable/disable skinning.  
        // May decide (in optimize.cpp) to put all with 1 bone in a different strip 
        // than those that need skinning.
        short numBones;

        unsigned char flags;

        int numBoneStateChanges;
        int boneStateChangeOffset;
        inline BoneStateChangeHeader_t *pBoneStateChange(int i) const
        {
            return (BoneStateChangeHeader_t *) (((byte *) this) + boneStateChangeOffset) + i;
        };
    };

    enum StripGroupFlags_t
    {
        STRIPGROUP_IS_FLEXED = 0x01,
        STRIPGROUP_IS_HWSKINNED = 0x02,
        STRIPGROUP_IS_DELTA_FLEXED = 0x04,
        STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
    };

    // a locking group
    // a single vertex buffer
    // a single index buffer
    struct StripGroupHeader_t
    {
        // These are the arrays of all verts and indices for this mesh.  strips index into this.
        int numVerts;
        int vertOffset;
        inline Vertex_t *pVertex(int i) const
        {
            return (Vertex_t *) (((byte *) this) + vertOffset) + i;
        };

        int numIndices;
        int indexOffset;
        inline unsigned short *pIndex(int i) const
        {
            return (unsigned short *) (((byte *) this) + indexOffset) + i;
        };

        int numStrips;
        int stripOffset;
        inline StripHeader_t *pStrip(int i) const
        {
            return (StripHeader_t *) (((byte *) this) + stripOffset) + i;
        };

        unsigned char flags;
    };

    enum MeshFlags_t
    {
        // these are both material properties, and a mesh has a single material.
        MESH_IS_TEETH = 0x01,
        MESH_IS_EYES = 0x02
    };

    // a collection of locking groups:
    // up to 4:
    // non-flexed, hardware skinned
    // flexed, hardware skinned
    // non-flexed, software skinned
    // flexed, software skinned
    //
    // A mesh has a material associated with it.
    struct MeshHeader_t
    {
        int numStripGroups;
        int stripGroupHeaderOffset;
        inline StripGroupHeader_t *pStripGroup(int i) const
        {
            StripGroupHeader_t *pDebug = (StripGroupHeader_t *) (((byte *) this) + stripGroupHeaderOffset) + i;
            return pDebug;
        };
        unsigned char flags;
    };

    struct ModelLODHeader_t
    {
        int numMeshes;
        int meshOffset;
        float switchPoint;
        inline MeshHeader_t *pMesh(int i) const
        {
            MeshHeader_t *pDebug = (MeshHeader_t *) (((byte *) this) + meshOffset) + i;
            return pDebug;
        };
    };

    // This maps one to one with models in the mdl file.
    // There are a bunch of model LODs stored inside potentially due to the qc $lod command
    struct ModelHeader_t
    {
        int numLODs; // garymcthack - this is also specified in FileHeader_t
        int lodOffset;
        inline ModelLODHeader_t *pLOD(int i) const
        {
            ModelLODHeader_t *pDebug = (ModelLODHeader_t *) (((byte *) this) + lodOffset) + i;
            return pDebug;
        };
    };

    struct BodyPartHeader_t
    {
        int numModels;
        int modelOffset;
        inline ModelHeader_t *pModel(int i) const
        {
            ModelHeader_t *pDebug = (ModelHeader_t *) (((byte *) this) + modelOffset) + i;
            return pDebug;
        };
    };

    struct MaterialReplacementHeader_t
    {
        short materialID;
        int replacementMaterialNameOffset;
        inline const char *pMaterialReplacementName(void)
        {
            const char *pDebug = (const char *) (((byte *) this) + replacementMaterialNameOffset);
            return pDebug;
        }
    };

    struct MaterialReplacementListHeader_t
    {
        int numReplacements;
        int replacementOffset;
        inline MaterialReplacementHeader_t *pMaterialReplacement(int i) const
        {
            MaterialReplacementHeader_t *pDebug = (MaterialReplacementHeader_t *) (((byte *) this) + replacementOffset) + i;
            return pDebug;
        }
    };

    struct FileHeader_t
    {
        // file version as defined by OPTIMIZED_MODEL_FILE_VERSION
        int version;

        // hardware params that affect how the model is to be optimized.
        int vertCacheSize;
        unsigned short maxBonesPerStrip;
        unsigned short maxBonesPerTri;
        int maxBonesPerVert;

        // must match checkSum in the .mdl
        int checkSum;

        int numLODs; // garymcthack - this is also specified in ModelHeader_t and should match

        // one of these for each LOD
        int materialReplacementListOffset;
        MaterialReplacementListHeader_t *pMaterialReplacementList(int lodID) const
        {
            MaterialReplacementListHeader_t *pDebug =
                (MaterialReplacementListHeader_t *) (((byte *) this) + materialReplacementListOffset) + lodID;
            return pDebug;
        }

        int numBodyParts;
        int bodyPartOffset;
        inline BodyPartHeader_t *pBodyPart(int i) const
        {
            BodyPartHeader_t *pDebug = (BodyPartHeader_t *) (((byte *) this) + bodyPartOffset) + i;
            return pDebug;
        };
    };
#pragma pack()
}

struct Vertex;

class MDLReader
{

public:
    MDLReader();
    bool LoadMDL(const char *pPath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

protected:
    bool ReadVTX(std::vector<uint32_t>& indices);
    bool ReadVVD(std::vector<Vertex> &vertices);

private:
    std::string m_BasePath;
    void PrintVertex(VVD::mstudiovertex_t &vert);
    void PrintHeader(VVD::vertexFileHeader_t &header);
    void PrintHeader(VTX::FileHeader_t &header);
};
