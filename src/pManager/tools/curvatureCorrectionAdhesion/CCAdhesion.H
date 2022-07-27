#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <regex>
#include <fstream>

#include <vector>

#include "triSurface.H"


#define _USE_MATH_DEFINES
#include <cmath>

#ifndef CCAdhesion_H_
#define CCAdhesion_H_


class CurCAdhesionForce {
	/*
	 * initialise this class with following args:
	 * -filepath of .stl formated surface file (relative to executing directory)
	 * -Hamaker constant of your used material?? TODO
	 * -the pradicted valence (default value is 6, that means up to 5 neighbourtriangles is predicted)
	 *
	 * initialiseing this class:
	 * ->reads the file
	 * ->stores all area normalvectors (double* normals_[])
	 * ->stores all triangle edge points (double* tris_[])
	 * ->creates a hastalble with a hashvalue for each point in the read data
	 * ->stores curvature (double* curve[]) (angleDeficitMethod is used)
	 *
	 * field-index i (e.g. normals_[i]) is following the .stl order (from the top)
	 * so index i is i-th normalvector and tris_[i:i+2] are corresponring triangleedges
	 */

public:
	//double* normals_;
	std::vector<double>* normals_;
	//double* tris_;
	std::vector<double>* tris_;
	int valence_;
	//double* hashTableCoords;
	std::vector<double>* hashTableCoords;
	double* curve;
private:
	int tableSize_;
	int* hashTableTriNum;
	int jump;

	//BETA
	std::string out_;
public:
	CurCAdhesionForce(triSurface& triSurf , int valence=6){

		jump = 3;	//needed to iterate over hashset-->3
		const int triSurfSize = triSurf.size();


		normals_ = new std::vector <double> [triSurfSize];	//coordinates for  normalvector
		tris_ = new std::vector <double> [triSurfSize*3];	//coordiantes for triangle points

		tableSize_ = triSurfSize*4;	//total amount of vectors (normal, edge)
		hashTableCoords = new std::vector <double> [tableSize_]; //*3 TODO
		hashTableTriNum = new int[tableSize_*valence_];


		curve = new double[triSurfSize];
		generateDataStructure(triSurf);
		zeroHashTable();
		sortIntoHashTable();
		calcCurves();

		delete normals_, tris_, hashTableCoords, hashTableTriNum, curve;
	}


	// Transforms the data given by Foam::triSurf into
	// more common std structures used in this code
	void generateDataStructure(triSurf& triSurf) {

		for(int i = 0; i < normals_; i++) {
			normals_[i]	= { triSurf.normals()[i].x(), triSurf.normals()[i].y(), triSurf.normals()[i].z() };
			tris_[i]	= { triSurf.points()[(triSurf.pointFaces()[i])[0]],
							triSurf.points()[(triSurf.pointFaces()[i])[1]]
							triSurf.points()[(triSurf.pointFaces()[i])[2]] }
			};
		}
	}


	void triTest(){
		for (int i; i<triasCount; i++){
			std::cout << normals_[i][0] << std::endl;
		}
	}
	// This is a Test
	void zeroHashTable(){
		/* function creates a hastable thats size is big enough to store curvature data
		 * valence is max amount of neighbourspoints (-1) for any point
		 * size is 4 times the nuber of trias (nuber of all vectors needed)
		 * multiplied by three (each vector has 3 coordinates)
		 */

		hashTableTriNum = new int[tableSize_*valence_];
		for( int i=0; i<tableSize_; i++){
			hashTableCoords[i] = {0,0,0};
		}
		for( int i=0; i<tableSize_*valence_; i++){
				for(int j=0; j<3; j++){
					hashTableTriNum[i] = -1;
				}
		}

	}

private:

	int hashing(std::vector<double> hashVector){
		/*
		 * creates dependig on x,y,z coordinates a hashvalue used as an identification
		 * -intput x, y, z coordinates of a point
		 * -returns hashvalue ID
		 * -makes sure there are no positions saved twice
		 */
		double x = hashVector[0];
		double y = hashVector[1];
		double z = hashVector[2];
		return (((unsigned int)(x*tableSize_))^((unsigned int)(y*2*tableSize_))^((unsigned int)(z*4*tableSize_))) % tableSize_;
	}

	int getPos(std::vector<double> posVector){
		int pos = hashing(posVector);
		while(true){
			if(hashTableCoords[pos][0] == posVector[0] && hashTableCoords[pos][1] == posVector[1] && hashTableCoords[pos][2] == posVector[2]){
				return pos;
			}
			pos = ((pos+jump)%tableSize_);
		}
	}

	void addCoordToHash(std::vector<double> newVector, int num){
		int pos = hashing(newVector);
		while(true){
			if(hashTableTriNum[valence_*pos + 0] == -1){
				hashTableCoords[pos][0] = newVector[0];
				hashTableCoords[pos][1] = newVector[1];
				hashTableCoords[pos][2] = newVector[2];
				hashTableTriNum[valence_*pos + 0] = num;
				return;
			}else if(hashTableCoords[pos][0] == newVector[0] && hashTableCoords[pos][1] == newVector[1] && hashTableCoords[pos][2] == newVector[2]){
				for(int i=1; i<valence_; i++){
					if(hashTableTriNum[valence_*pos + i] == -1){
						hashTableTriNum[valence_*pos + i] = num;
						return;
					}
				}
				std::cerr<<"valence_ nicht hoch genug, Dreieck "<<num<<" kann nicht einsortiert werden:\t"<<newVector[0]<<"\t"<<newVector[1]<<"\t"<<newVector[2]<<std::endl;
			}
			pos = ((pos+jump)%tableSize_);
		}
	}

public:
	void sortIntoHashTable(){
		/*
		 * sorts read coordinates in a hashtable
		 */

		for(int i=0; i<triasCount; i++){
			for(int j=0; j<3; j++){
				//addCoordToHash(tris_[9*i+3*j+0], tris_[9*i+3*j+1], tris_[9*i+3*j+2], i);
				addCoordToHash(tris_[3*i+j], i);
			}
		}
	}

private:
	double CurvatureAngleDeficitMethod(int pos){
		/*
		 * calculates curvature for every element using "angle deficit method":
		 * 		-calculates curvature at every point read from .stl file
		 * 		-calculates curvature for every element as an average of curvature at
		 * 			cornerpoints
		 */
		double angleSum = 2* M_PI;
		double areaSum = 0;
		std::vector <double> referenceVector = hashTableCoords[pos];

		int j=0;

		std::vector <double> vectorA;
		std::vector <double> vectorB;

		while(j<valence_ && hashTableTriNum[valence_*pos + j] != -1){
			int switchHelper = -1;
	//for each triangle, find the center point, then move the center point to the origin of the coordinate system
			if(tris_[3*hashTableTriNum[valence_*pos + j]] == referenceVector){
				switchHelper=1;
			}else if(tris_[3*hashTableTriNum[valence_*pos + j]+1] == referenceVector){
				switchHelper=2;
			}else if(tris_[3*hashTableTriNum[valence_*pos + j]+2] == referenceVector){
				switchHelper=3;
			}else{
				switchHelper = 0;
			}

			switch(switchHelper)
			{
			case(0):
					std::cout<<"Fehler gleiche Koordinaten lassen sich nicht finden"<<std::endl;
					break;
			case(1):
					vectorA = tris_[3*hashTableTriNum[valence_*pos + j] +1];
					vectorB = tris_[3*hashTableTriNum[valence_*pos + j] +2];
					break;
			case(2):
					vectorA = tris_[3*hashTableTriNum[valence_*pos + j]];
					vectorB = tris_[3*hashTableTriNum[valence_*pos + j]+2];
					break;
			case(3):
					vectorA = tris_[3*hashTableTriNum[valence_*pos + j]+1];
					vectorB = tris_[3*hashTableTriNum[valence_*pos + j]];
					break;
			}
			//create relative Vector (to given in arg)
			for(int i=0; i<3;i++){
				vectorA[i] -= referenceVector[i];
				vectorB[i] -= referenceVector[i];
			}



			double kreuzX = (vectorA[1] * vectorB[2]) - (vectorA[2] * vectorB[1]);
			double kreuzY = (vectorA[2] * vectorB[0]) - (vectorA[0] * vectorB[2]);
			double kreuzZ = (vectorA[0] * vectorB[1]) - (vectorA[1] * vectorB[0]);

			double aN = sqrt(vectorA[0]*vectorA[0] + vectorA[1]*vectorA[1] + vectorA[2]*vectorA[2]);
			double bN = sqrt(vectorB[0]*vectorB[0] + vectorB[1]*vectorB[1] + vectorB[2]*vectorB[2]);
			double kN = sqrt(kreuzX*kreuzX + kreuzY*kreuzY + kreuzZ*kreuzZ);
	//calculate the area of each triangle and the angle theta
			double angle = asin(kN/(aN*bN));
			double area = kN; // doppelte area, faktor wird anchher beim return reinmultipliziert

			angleSum -= angle;
			areaSum += area;

			j++;
		}
		return 2*3*angleSum/areaSum;
	}

	double correctionMatlab1(double area, double curve, double distanceSQ, double normalProd){
		return (1.0 + 4.246e14*pow(area*curve, 1.71)*pow(area/sqrt(distanceSQ*curve), 0.931));
	}
	double correctionMatlab2(double area, double curve, double distanceSQ, double normalProd){
		return (1+pow(area*curve, 1.678850523395267) *(2.862285375829145e+15*pow(area/sqrt(distanceSQ*curve),0.922080200960708)*pow(3-1*normalProd,-3)));
	}
	double correctionMatlab3(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 8.576832288407436e+13 * pow(area*curve, 1.678850523395267) * pow(area/sqrt(distanceSQ*curve),0.922080200960708)*pow(3.612623450755996-0.332229534762994*normalProd,1.152488104175228));
	}
	double correctionMatlab4(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 1.493452081530021e+17* pow(area*curve, 1.43080504240259) * pow(area/sqrt(distanceSQ*curve),0.967757922416126)*pow(1-normalProd,0.5));
	}
	double correctionMatlab5(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 930.942249599793*3.602836375165311e+14* pow(area*curve, 0.979515115057189) * pow(area/sqrt(distanceSQ*curve),0.972994322720961)*pow(1-normalProd,1));
	}
	double correctionMatlab6(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 2212.43950163744*3.602836375165311e+14* pow(area*curve, 0.00745591895316777) * pow(area/sqrt(distanceSQ*curve),1.00319167786467)*pow(1-normalProd,2));
	}
	double correctionMatlab7(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 1389.13335225059 *3.602836375165311e+14* pow(area*curve, 0.492583924987022) * pow(area/sqrt(distanceSQ*curve),0.977856665471021)*pow(1-normalProd,1.5));
	}
	double correctionMatlab8(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 1779.12613487437 *3.602836375165311e+14* pow(area*curve, -0.723141061359918) * pow(area/sqrt(distanceSQ*curve),1.00903881851439)*pow(1-normalProd,3));
	}
	double correctionMatlab9(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 1056.57394780272 *3.602836375165311e+14* pow(area*curve, -0.839585053759633) * pow(area/sqrt(distanceSQ*curve),0.979810082287797)*pow(1-normalProd,3.5));
	}
	double correctionMatlab10(double area, double curve, double distanceSQ, double normalProd){
		return (1 + 0.000035073378890 *3.602836375165311e+14* pow(area*curve, -0.839585053759633) * pow(area/sqrt(distanceSQ*curve),0.979810082287797)*pow(1-normalProd,3.5)*pow(distanceSQ, 2.162413032414761));
	}

public:

	void calcCurves(){
		/*
		 * name:Test
		 */
		for(int i=0; i<triasCount; i++){
			curve[i] = 0.0;
			for(int j=0; j<3; j++){
				int pos = hashing(tris_[3*i +j]);
				//int pos = getPos(tris_[3*i +j]);
				curve[i] += CurvatureAngleDeficitMethod(pos);
			}
			curve[i] /= 3.0;
		}
	}
	double ElementCurvature(int triangleNumber){
		/*
		 * returns curvature (double curve[triangleNumber]) of the "triangleNumber" element of .STL file
		 */
		return curve[triangleNumber];
	}

};



#endif /* CURCADHESIONFORCE_H_ */

