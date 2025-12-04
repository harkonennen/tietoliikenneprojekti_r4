#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"
#include "centers.h"

int CM[6][6]= {0};

void printConfusionMatrix(void)
{
	printk("Confusion matrix = \n");
	printk("   cp1 cp2 cp3 cp4 cp5 cp6\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d   %d   %d   %d\n",i+1,CM[i][0],CM[i][1],CM[i][2],CM[i][3],CM[i][4],CM[i][5]);
	}
}

void printCenters(void)
{
	printk("Centers = \n");
	printk("   cp1 cp2 cp3\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d\n",i+1,centers[i][0],centers[i][1],centers[i][2]);
	}
}


void makeOneClassificationAndUpdateConfusionMatrix(int direction)
{
  for(int i = 0; i < 100; i++){

	struct Measurement m = readADCValue();
	printk("x = %d,  y = %d,  z = %d\n",m.x,m.y,m.z);

	int winner = calculateDistanceToAllCentrePointsAndSelectWinner(m.x,m.y,m.z);

   printk("winner %d \n",winner);
   printk("direction %d \n",direction);

   CM[direction][winner]+=1;
  }
}

int calculateDistanceToAllCentrePointsAndSelectWinner(int x,int y,int z)
{
  int distances[6] = {};

   for (int i = 0; i < 6; i++){
      int diffx = x - centers[i][0];
      int diffy = y - centers[i][1];
      int diffz = z -centers[i][2];

      int sumx = diffx * diffx;
      int sumy = diffy * diffy;
      int sumz = diffz * diffz;

      int total = sqrt(sumx + sumy +sumz);
      distances[i] = total;
   }

   //for (int i = 0; i < 6; i++){
   //   printf("cp%d %d   %d  %d\n",i+1,distances[i],distances[i],distances[i]);
   //}


//int minDistance = distances[0];

int closestIndex = 0;
for (int i = 1; i < 6; i++) {
   if (distances[i] < distances[closestIndex]) {
      closestIndex = i;
   }
}

//printk("minimum distance %d and index %d \n",distances[closestIndex],closestIndex);
   return closestIndex;
}

void resetConfusionMatrix(void)
{
	for(int i=0;i<6;i++)
	{ 
		for(int j = 0;j<6;j++)
		{
			CM[i][j]=0;
		}
	}
}

