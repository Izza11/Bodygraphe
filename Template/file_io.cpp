#include <fstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "file_io.h"
#include "LoadTexture.h"

extern std::vector<std::vector<std::string>> background_filenames;
extern std::vector<std::vector<int>> background_textures;

struct gen_file
{
   int type;
   glm::vec4 params;
};

std::ostream &operator<< (std::ostream &out, const glm::mat4 &M)
{
   out << "mat4 ";
   for (int i = 0; i<4; i++)
   {
      out << M[i].x << " " << M[i].y << " " << M[i].z << " " << M[i].w << " ";
   }
   return out;
}

std::istream& operator>> (std::istream &in, glm::mat4 &M)
{
   std::string header; //should be "mat4"
   in >> header;
   for (int i = 0; i<4; i++)
   {
      in >> M[i].x >> M[i].y >> M[i].z >> M[i].w;
   }

   return in;
}

std::ostream &operator<< (std::ostream &out, const glm::vec3 &v)
{
   out << "vec3 ";
   out << v.x << " " << v.y << " " << v.z << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::vec3 &v)
{
   std::string header; //should be "vec3"
   in >> header;
   in >> v.x >> v.y >> v.z;

   return in;
}

std::ostream &operator<< (std::ostream &out, const glm::ivec3 &v)
{
   out << "ivec3 ";
   out << v.x << " " << v.y << " " << v.z << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::ivec3 &v)
{
   std::string header; //should be "ivec3"
   in >> header;
   in >> v.x >> v.y >> v.z;

   return in;
}

std::ostream &operator<< (std::ostream &out, const glm::vec4 &v)
{
   out << "vec4 ";
   out << v.x << " " << v.y << " " << v.z << " " << v.w << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::vec4 &v)
{
   std::string header; //should be "vec4"
   in >> header;
   in >> v.x >> v.y >> v.z >> v.w;

   return in;
}


std::ostream &operator<< (std::ostream &out, const glm::quat &v)
{
   out << "quat ";
   out << v.x << " " << v.y << " " << v.z << " " << v.w << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::quat &v)
{
   std::string header; //should be "quat"
   in >> header;
   in >> v.x >> v.y >> v.z >> v.w;

   return in;
}

std::ostream &operator<< (std::ostream &out, const gen_file &g)
{
   out << "GEN_FILE ";
   out << g.type << " " << g.params << " ";
   out << std::endl;
   return out;
}

std::istream& operator>> (std::istream &in, gen_file &g)
{
   std::string header; //should be "GEN_FILE "
   in >> header;
   in >> g.type >> g.params;

   return in;
}


void file_load(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::in);

   std::string header;

   std::string id;

   background_filenames.resize(10);
   background_textures.resize(10);

   int key = 0;
   std::string filename;

   while(!fs.eof())
   {
		fs >> id;
		if(id=="key")
		{
			fs >> key;
		}

		if(id=="file")
		{
			fs >> filename;
			background_filenames[key].push_back(filename);
			int tex_id = LoadTexture(filename);
			background_textures[key].push_back(tex_id);
		}

   }
   
   fs.close();
}

void file_save(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::out);

   /*
   fs << bldgs.size();
   fs << std::endl;

   for (auto &b : bldgs)
   {
      fs << b << spc;
   }
   fs << std::endl;
   */

   fs.close();
}

