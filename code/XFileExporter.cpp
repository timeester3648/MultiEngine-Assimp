/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team
All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the 
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@author: Richard Steffen, 2014
----------------------------------------------------------------------
*/

#include "AssimpPCH.h"

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_XFILE_EXPORTER
#include "XFileExporter.h"
#include "ConvertToLHProcess.h"
#include "Bitmap.h"
#include "fast_atof.h"
#include "SceneCombiner.h" 

#include <ctime>
#include <set>

using namespace Assimp;

namespace Assimp
{

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Collada. Prototyped and registered in Exporter.cpp
void ExportSceneXFile(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene)
{
	std::string path = "";
	std::string file = pFile;

	// We need to test both types of folder separators because pIOSystem->getOsSeparator() is not reliable.
	// Moreover, the path given by some applications is not even consistent with the OS specific type of separator.
	const char* end_path = std::max(strrchr(pFile, '\\'), strrchr(pFile, '/'));

	if(end_path != NULL) {
		path = std::string(pFile, end_path + 1 - pFile);
		file = file.substr(end_path + 1 - pFile, file.npos);

		std::size_t pos = file.find_last_of('.');
		if(pos != file.npos) {
			file = file.substr(0, pos);
		}
	}

	// invoke the exporter 
	XFileExporter iDoTheExportThing( pScene, pIOSystem, path, file);

	// we're still here - export successfully completed. Write result to the given IOSYstem
	boost::scoped_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
	if(outfile == NULL) {
		throw DeadlyExportError("could not open output .dae file: " + std::string(pFile));
	}

	// XXX maybe use a small wrapper around IOStream that behaves like std::stringstream in order to avoid the extra copy.
	outfile->Write( iDoTheExportThing.mOutput.str().c_str(), static_cast<size_t>(iDoTheExportThing.mOutput.tellp()),1);
}

} // end of namespace Assimp


// ------------------------------------------------------------------------------------------------
// Constructor for a specific scene to export
XFileExporter::XFileExporter(const aiScene* pScene, IOSystem* pIOSystem, const std::string& path, const std::string& file) : mIOSystem(pIOSystem), mPath(path), mFile(file)
{
	// make sure that all formatting happens using the standard, C locale and not the user's current locale
	mOutput.imbue( std::locale("C") );

	mScene = pScene;
	mSceneOwned = false;

	// set up strings
	endstr = "\n"; 

	// start writing
	WriteFile();
}

// ------------------------------------------------------------------------------------------------
// Destructor
XFileExporter::~XFileExporter()
{
	if(mSceneOwned) {
		delete mScene;
	}
}

// ------------------------------------------------------------------------------------------------
// Starts writing the contents
void XFileExporter::WriteFile()
{
	// note, that all realnumber values must be comma separated in x files
	mOutput.setf(std::ios::fixed);
	mOutput.precision(6); // precission for float

	// entry of writing the file
	WriteHeader();

	mOutput << startstr << "Frame DXCC_ROOT {" << endstr;
	PushTag();

	aiMatrix4x4 I; // identity
	WriteFrameTransform(I);

	WriteNode(mScene->mRootNode);
	PopTag();

	mOutput << startstr << "}" << endstr;

}

// ------------------------------------------------------------------------------------------------
// Writes the asset header
void XFileExporter::WriteHeader()
{
	mOutput << startstr << "xof 0303txt 0032" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template Frame {" << endstr;
	PushTag();
	mOutput << startstr << "<3d82ab46-62da-11cf-ab39-0020af71e433>" << endstr;
	mOutput << startstr << "[...]" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template Matrix4x4 {" << endstr;
	PushTag();
	mOutput << startstr << "<f6f23f45-7686-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "array FLOAT matrix[16];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template FrameTransformMatrix {" << endstr;
	PushTag();
	mOutput << startstr << "<f6f23f41-7686-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "Matrix4x4 frameMatrix;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template Vector {" << endstr;
	PushTag();
	mOutput << startstr << "<3d82ab5e-62da-11cf-ab39-0020af71e433>" << endstr;
	mOutput << startstr << "FLOAT x;" << endstr;
	mOutput << startstr << "FLOAT y;" << endstr;
	mOutput << startstr << "FLOAT z;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template MeshFace {" << endstr;
	PushTag();
	mOutput << startstr << "<3d82ab5f-62da-11cf-ab39-0020af71e433>" << endstr;
	mOutput << startstr << "DWORD nFaceVertexIndices;" << endstr;
	mOutput << startstr << "array DWORD faceVertexIndices[nFaceVertexIndices];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template Mesh {" << endstr;
	PushTag();
	mOutput << startstr << "<3d82ab44-62da-11cf-ab39-0020af71e433>" << endstr;
	mOutput << startstr << "DWORD nVertices;" << endstr;
	mOutput << startstr << "array Vector vertices[nVertices];" << endstr;
	mOutput << startstr << "DWORD nFaces;" << endstr;
	mOutput << startstr << "array MeshFace faces[nFaces];" << endstr;
	mOutput << startstr << "[...]" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template MeshNormals {" << endstr;
	PushTag();
	mOutput << startstr << "<f6f23f43-7686-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "DWORD nNormals;" << endstr;
	mOutput << startstr << "array Vector normals[nNormals];" << endstr;
	mOutput << startstr << "DWORD nFaceNormals;" << endstr;
	mOutput << startstr << "array MeshFace faceNormals[nFaceNormals];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template Coords2d {" << endstr;
	PushTag();
	mOutput << startstr << "<f6f23f44-7686-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "FLOAT u;" << endstr;
	mOutput << startstr << "FLOAT v;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template MeshTextureCoords {" << endstr;
	PushTag();
	mOutput << startstr << "<f6f23f40-7686-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "DWORD nTextureCoords;" << endstr;
	mOutput << startstr << "array Coords2d textureCoords[nTextureCoords];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template ColorRGBA {" << endstr;
	PushTag();
	mOutput << startstr << "<35ff44e0-6c7c-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "FLOAT red;" << endstr;
	mOutput << startstr << "FLOAT green;" << endstr;
	mOutput << startstr << "FLOAT blue;" << endstr;
	mOutput << startstr << "FLOAT alpha;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template IndexedColor {" << endstr;
	PushTag();
	mOutput << startstr << "<1630b820-7842-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "DWORD index;" << endstr;
	mOutput << startstr << "ColorRGBA indexColor;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template MeshVertexColors {" << endstr;
	PushTag();
	mOutput << startstr << "<1630b821-7842-11cf-8f52-0040333594a3>" << endstr;
	mOutput << startstr << "DWORD nVertexColors;" << endstr;
	mOutput << startstr << "array IndexedColor vertexColors[nVertexColors];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template VertexElement {" << endstr;
	PushTag();
	mOutput << startstr << "<f752461c-1e23-48f6-b9f8-8350850f336f>" << endstr;
	mOutput << startstr << "DWORD Type;" << endstr;
	mOutput << startstr << "DWORD Method;" << endstr;
	mOutput << startstr << "DWORD Usage;" << endstr;
	mOutput << startstr << "DWORD UsageIndex;" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
	mOutput << startstr << "template DeclData {" << endstr;
	PushTag();
	mOutput << startstr << "<bf22e553-292c-4781-9fea-62bd554bdd93>" << endstr;
	mOutput << startstr << "DWORD nElements;" << endstr;
	mOutput << startstr << "array VertexElement Elements[nElements];" << endstr;
	mOutput << startstr << "DWORD nDWords;" << endstr;
	mOutput << startstr << "array DWORD data[nDWords];" << endstr;
	PopTag();
	mOutput << startstr << "}" << endstr;
	mOutput << endstr;
}


// Writes the material setup
void XFileExporter::WriteFrameTransform(aiMatrix4x4& m)
{
	mOutput << startstr << "FrameTransformMatrix {" << endstr << " ";
	PushTag();
	mOutput << startstr << m.a1 << ", " << m.b1 << ", " << m.c1 << ", " << m.d1 << "," << endstr;
    mOutput << startstr << m.a2 << ", " << m.b2 << ", " << m.c2 << ", " << m.d2 << "," << endstr;
	mOutput << startstr << m.a3 << ", " << m.b3 << ", " << m.c3 << ", " << m.d3 << "," << endstr;
	mOutput << startstr << m.a4 << ", " << m.b4 << ", " << m.c4 << ", " << m.d4 << ";;" << endstr;		
	PopTag();
	mOutput << startstr << "}" << endstr << endstr;
}


// ------------------------------------------------------------------------------------------------
// Recursively writes the given node
void XFileExporter::WriteNode( aiNode* pNode)
{	
	if (pNode->mName.length==0)
	{
		std::stringstream ss;
		ss << "Node_" << pNode;
		pNode->mName.Set(ss.str());
	}
	mOutput << startstr << "Frame " << pNode->mName.C_Str() << " {" << endstr;

	PushTag();

	aiMatrix4x4 m = pNode->mTransformation;

	WriteFrameTransform(m);

	for (size_t i = 0; i < pNode->mNumMeshes; i++)
		WriteMesh(mScene->mMeshes[pNode->mMeshes[i]]);

	// recursive call the Nodes
	for (size_t a = 0; a < pNode->mNumChildren; a++)
		WriteNode( mScene->mRootNode->mChildren[a]);

	PopTag();

	mOutput << startstr << "}" << endstr << endstr;
}

void XFileExporter::WriteMesh(const aiMesh* mesh)
{
	mOutput << startstr << "Mesh " << mesh->mName.C_Str() << "_mShape" << " {" << endstr;

	PushTag();

	// write all the vertices
	mOutput << startstr << mesh->mNumVertices << ";" << endstr;	
	for (size_t a = 0; a < mesh->mNumVertices; a++)
	{
		aiVector3D &v = mesh->mVertices[a];
		mOutput << startstr << v[0] << ";"<< v[1] << ";" << v[2] << ";";
		if (a < mesh->mNumVertices - 1)
			mOutput << "," << endstr;
		else
			mOutput << ";" << endstr;
	}

	// write all the faces
	mOutput << startstr << mesh->mNumFaces << ";" << endstr;
	for( size_t a = 0; a < mesh->mNumFaces; ++a )
	{
		const aiFace& face = mesh->mFaces[a];
		mOutput << startstr << face.mNumIndices << ";";
		// must be counter clockwise triangle
		//for(int b = face.mNumIndices - 1; b >= 0 ; --b)
		for(size_t b = 0; b < face.mNumIndices ; ++b)
		{
			mOutput << face.mIndices[b];
			//if (b > 0)
			if (b<face.mNumIndices-1)
				mOutput << ",";
			else
				mOutput << ";";
		}

		if (a < mesh->mNumFaces - 1)
			mOutput << "," << endstr;
		else
			mOutput << ";" << endstr;
	}

	mOutput << endstr;

	if (mesh->HasTextureCoords(0))
	{
		const aiMaterial* mat = mScene->mMaterials[mesh->mMaterialIndex];
		aiString relpath;
		mat->Get(_AI_MATKEY_TEXTURE_BASE, aiTextureType_DIFFUSE, 0, relpath);		

		mOutput << startstr << "MeshMaterialList {" << endstr;
		PushTag();
		mOutput << startstr << "1;" << endstr; // number of materials
		mOutput << startstr << mesh->mNumFaces << ";" << endstr; // number of faces
		mOutput << startstr;
		for( size_t a = 0; a < mesh->mNumFaces; ++a )
		{		
			mOutput << "0"; // the material index
			if (a < mesh->mNumFaces - 1)
				mOutput << ", ";
			else
				mOutput << ";" << endstr;
		}		
		mOutput << startstr << "Material {" << endstr;	
		PushTag();
		mOutput << startstr << "1.0; 1.0; 1.0; 1.000000;;" << endstr;
		mOutput << startstr << "1.000000;" << endstr; // power
		mOutput << startstr << "0.000000; 0.000000; 0.000000;;" << endstr; // specularity
		mOutput << startstr << "0.000000; 0.000000; 0.000000;;" << endstr; // emission
		mOutput << startstr << "TextureFilename { \"";

		writePath(relpath);		

		mOutput << "\"; }" << endstr;	
		PopTag();
		mOutput << startstr << "}" << endstr;
		PopTag();
		mOutput << startstr << "}" << endstr;	
	}

	// write normals (every vertex has one)
	if (mesh->HasNormals())
	{
		mOutput << endstr << startstr << "MeshNormals {" << endstr;
		mOutput << startstr << mesh->mNumVertices << ";" << endstr;
		for (size_t a = 0; a < mesh->mNumVertices; a++)
		{		
			aiVector3D &v = mesh->mNormals[a];
			// because we have a LHS and also changed wth winding, we need to invert the normals again
			mOutput << startstr << -v[0] << ";"<< -v[1] << ";" << -v[2] << ";";
			if (a < mesh->mNumVertices - 1)
				mOutput << "," << endstr;
			else
				mOutput << ";" << endstr;
		}

		mOutput << startstr << mesh->mNumFaces << ";" << endstr;
		for (size_t a = 0; a < mesh->mNumFaces; a++)
		{		
			const aiFace& face = mesh->mFaces[a];
			mOutput << startstr << face.mNumIndices << ";";

			//for(int b = face.mNumIndices-1; b >= 0 ; --b)
			for(size_t b = 0; b < face.mNumIndices ; ++b)
			{
				mOutput << face.mIndices[b];
				//if (b > 0)
				if (b<face.mNumIndices-1)
					mOutput << ",";
				else
					mOutput << ";";
			}

			if (a < mesh->mNumFaces-1)
				mOutput << "," << endstr;
			else
				mOutput << ";" << endstr;
		}
		mOutput << startstr << "}" << endstr;
	}
	
	// write texture UVs if available
	if (mesh->HasTextureCoords(0))
	{		
		mOutput << endstr << startstr << "MeshTextureCoords {"  << endstr;
		mOutput << startstr << mesh->mNumVertices << ";" << endstr;
		for (size_t a = 0; a < mesh->mNumVertices; a++)
		//for (int a = (int)mesh->mNumVertices-1; a >=0 ; a--)
		{
			aiVector3D& uv = mesh->mTextureCoords[0][a]; // uv of first uv layer for the vertex
			mOutput << startstr << uv.x << ";" << uv.y;
			if (a < mesh->mNumVertices-1)
			//if (a >0 )
				mOutput << ";," << endstr;
			else
				mOutput << ";;" << endstr;
		}
		mOutput << startstr << "}" << endstr;
	}

	// write color channel if available
	if (mesh->HasVertexColors(0))
	{		
		mOutput << endstr << startstr << "MeshVertexColors {"  << endstr;
		mOutput << startstr << mesh->mNumVertices << ";" << endstr;
		for (size_t a = 0; a < mesh->mNumVertices; a++)
		{
			aiColor4D& mColors = mesh->mColors[0][a]; // color of first vertex color set for the vertex
			mOutput << startstr << a << ";" << mColors.r << ";" << mColors.g << ";" << mColors.b << ";" << mColors.a << ";;";
			if (a < mesh->mNumVertices-1)
				mOutput << "," << endstr;
			else
				mOutput << ";" << endstr;
		}
		mOutput << startstr << "}" << endstr;
	}
	/*
	else
	{
		mOutput << endstr << startstr << "MeshVertexColors {"  << endstr;
		mOutput << startstr << mesh->mNumVertices << ";" << endstr;
		for (size_t a = 0; a < mesh->mNumVertices; a++)
		{
			aiColor4D* mColors = mesh->mColors[a];
			mOutput << startstr << a << ";0.500000;0.000000;0.000000;0.500000;;";
			if (a < mesh->mNumVertices-1)
				mOutput << "," << endstr;
			else
				mOutput << ";" << endstr;
		}
		mOutput << startstr << "}" << endstr;
	}
	*/
	PopTag();
	mOutput << startstr << "}" << endstr << endstr;

}


void XFileExporter::writePath(aiString path)
{
	std::string str = std::string(path.C_Str());
	BaseImporter::ConvertUTF8toISO8859_1(str);

	while( str.find( "\\\\") != std::string::npos)
		str.replace( str.find( "\\\\"), 2, "\\");

	while( str.find( "\\") != std::string::npos)
		str.replace( str.find( "\\"), 1, "/");

	mOutput << str;
			
}

#endif
#endif
