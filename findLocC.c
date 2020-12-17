#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define BC_PI ( 3.14159265358979323846 )
#define DEG2RAD(d) ( d * BC_PI / 180.0 )
#define RAD2DEG(d) ( d / BC_PI * 180.0 )

#define FINDLOCC_VERSION "v1.0.0"
#define FINDLOCC_COMPILED_DATE "Sep. 6, 2015"
#define FINDLOCC_COPYRIGHT "Copyright(c) 2015 by KIGAM All Rights Reserved"

//#define KOREA_CITY_INFO_FILE_NAME "korealocation_utf8"
#define KOREA_CITY_INFO_FILE_NAME "/home/sysop/KGOM/params/korealocation_utf8"
#define CITY_NAME_LEN_MAX (32)

typedef struct _koreaCityLoc
{
    float lat;
    float lon;
    char name[CITY_NAME_LEN_MAX];
} koreaCityLoc_t;

static int nKoreaCities;
static koreaCityLoc_t *koreaCityLocList;
static int myStrToDoubleError;


void mb_locdiff(double slat, double slon, double elat, double elon, double *distance, double *azim, double *bazim);
void mb_geocr(double lon, double lat, double *a, double *b, double *c);
double mb_azm(double x, double y);
void azim_dist(char *result, int maxResultSize, double lat, double lon);
void getLoc(char *result, int maxResultSize, double lat, double lon);

double mySrtToDouble(const char *str)
{
  int i = 0;
  double value;
  
  myStrToDoubleError = 0;
  
  if(str == NULL)
  {
    fprintf(stdout, "mySrtToDouble: Unexpected NULL argument: str\n");
    myStrToDoubleError = 1;
    return (0.0);
  }

  if((str[0] == '+') || (str[0] == '-'))
  {
    ++i;
  }

  if((str[i] < '0') || (str[i] > '9'))
  {
    fprintf(stdout, "mySrtToDouble: It is not a double type value: %s\n", str);
    myStrToDoubleError = 2;
    return (0.0);
  }

  errno = 0;
  value = strtod(str, NULL);
  if(errno)
  {
    fprintf(stdout, "mySrtToDouble: strtod() failed: %s\n", strerror(errno));
    myStrToDoubleError = 3;
    return (0.0);
  }

  return value;
  
}


void findLocC_usage(const char *cmd)
{

  fprintf(stdout, "%s %s compiled on %s\n", cmd, FINDLOCC_VERSION, FINDLOCC_COMPILED_DATE);
  fprintf(stdout, "%s\n", FINDLOCC_COPYRIGHT);
  fprintf(stdout, "Usage> %s [<latitude> <longitude> | -f <input_file>]\n", cmd);
  fprintf(stdout, "Example #1> %s 34.6683 127.058\n", cmd);
  fprintf(stdout, "Example #2> %s -f 2015212_2015218.list\n", cmd);
  fprintf(stdout, "Input file exmaple> \n");
  fprintf(stdout, "  UTC                 KST                     Lat      Lon            ML\n");
  fprintf(stdout, "  ----------------------------------------------------------------------\n");
  fprintf(stdout, "  2015/08/06 04:09:21 2015/08/06 13:09:21   38.8856  126.0476        2.1\n");
  fprintf(stdout, "  2015/08/02 10:06:41 2015/08/02 19:06:41   39.6083  124.9152        1.9\n");
  fprintf(stdout, "  ...\n");

  return;

}

int getCityInfo(const char *fileName)
{
  int nLine = 0;
  char line[256];

  FILE *fp = NULL;

  
  if(fileName == NULL)
  {
    fprintf(stdout, "getCityInfo: Unexpected NULL argument: fileName\n");
    return -1;
  }

  fp = fopen (fileName, "r");
  if(fp == NULL)
  {
    fprintf(stdout, "getCityInfo: Fail to open a file: %s\n", fileName);
    perror("fopen:");
    return -1;
  }

  while(fgets(line, 256, fp))
  {
    ++nLine;
  }

  koreaCityLocList = (koreaCityLoc_t *)calloc(nLine, sizeof (koreaCityLoc_t));
  if(koreaCityLocList == NULL)
  {
    fprintf(stdout, "getCityInfo: Calloc failed for koreaCityLocList\n");
    return -1;
  }

  fseek(fp, 0, SEEK_SET);
  while(fgets(line, 256, fp))
  {
    char *p = strtok(line, " \t\n");
    if(p == NULL)
    {
      continue;
    }

    koreaCityLocList[nKoreaCities].lat = mySrtToDouble(p);
    if(myStrToDoubleError)
    {
      continue;
    }

    p = strtok(NULL, " \t\n");
    if(p == NULL)
    {
      continue;
    }

    koreaCityLocList[nKoreaCities].lon = mySrtToDouble(p);
    if(myStrToDoubleError)
    {
      continue;
    }

    p = strtok(NULL, " \t\n");
    if(p == NULL)
    {
      continue;
    }

    strncpy(koreaCityLocList[nKoreaCities].name, (const char *)p, (CITY_NAME_LEN_MAX - 1));
    if(strlen(koreaCityLocList[nKoreaCities].name) <= 0)
    {
      continue;
    }

    p = strtok(NULL, " \t\n");
    if(p != NULL)
    {
      int strLen = strlen(koreaCityLocList[nKoreaCities].name);
      if (strLen < (CITY_NAME_LEN_MAX - 2))
      {
        koreaCityLocList[nKoreaCities].name[strLen++] = ' ';
        koreaCityLocList[nKoreaCities].name[strLen] = '\0';
        strncat(koreaCityLocList[nKoreaCities].name, (const char *)p, (CITY_NAME_LEN_MAX - strlen(koreaCityLocList[nKoreaCities].name) - 1));
      }
    }

    ++nKoreaCities;
    
  }

  return 0;

}


int main(int argc, char **argv)
{
	double	lat;
	double	lon;
	char*	fileName = NULL;
  char result[128] = {0x00};

  if(argc != 3)
  {
    char *ch = strrchr(argv[0], '/');
    if(ch == NULL) 
    {
      ch = argv[0];
    }
    else
    {
      ++ch;
    }
    findLocC_usage(ch);
    return -1;
  }

  if(strcmp (argv[1], "-f") == 0)
  {
    fileName = argv[2];
  }

  if(getCityInfo(KOREA_CITY_INFO_FILE_NAME) < 0)
  {
    fprintf (stdout, "Fail to get city's information from %s\n", KOREA_CITY_INFO_FILE_NAME);
  }

  if(fileName)
  {
    FILE *stream = NULL;
    FILE *output = NULL;
    char line[256] = {0x00};

    stream = fopen(fileName, "r");
    if(stream == NULL)
    {
      fprintf(stdout, "File open failed: %s\n", fileName);
      perror("fopen: ");
      return -1;
    }

    strncpy(line, fileName, sizeof (line));
    strcat(line, ".result");
    output = fopen(line, "w");
    if(stream == NULL)
    {
      fprintf(stdout, "File open failed: %s\n", line);
      perror("fopen: ");
      return -1;
    }

    while (fgets(line, sizeof (line), stream) != NULL) 
    {
      char *p = NULL;
      int len = strlen(line);
      if(line[(len - 1)] == '\n') /* Remove endline character */
      {
        line[(len - 1)] = '\0';
      }
      fprintf (output, "\n%s", line);
      
      /* Parse the line */
      p = strtok(line, " \t\n");
      p = strtok(NULL, " \t\n");
      p = strtok(NULL, " \t\n");
      p = strtok(NULL, " \t\n");
      p = strtok(NULL, " \t\n");
      if(p == NULL)
      {
        continue;
      }
      
      lat = mySrtToDouble ((const char *)p);
      if (myStrToDoubleError)
      {
        continue;
      }
      
      p = strtok(NULL, " \t\n");
      if(p == NULL)
      {
        continue;
      }
      
      lon = mySrtToDouble ((const char *)p);
      if (myStrToDoubleError)
      {
        continue;
      }

      if((lat != 0.0) || (lon != 0.0))
      {
        /* Get a location */
        getLoc (result, (int)sizeof (result), lat, lon);
        //fprintf (stdout, "%f, %f = %s\n", lat, lon, result);
        fprintf (stdout, "%s\n", result);
        fprintf (output, "\t%s", result);

        #if 0
        /* Append the result in the line */
        len = strlen(result);
        fseek (stream, -1, SEEK_CUR);
        if(fwrite ((void *)result, len, 1, stream) != 1)
        {
          fprintf (stdout, "Fail to write a result into the file\n.");
          goto file_operation_end;
        }
        fflush(stream);
        #endif
      }
    }

    file_operation_end:
    fclose(stream);
    fclose(output);
  }
  else
  {
    lat = mySrtToDouble ((const char *)argv[1]);
    if (myStrToDoubleError)
    {
      fprintf(stdout, "Invalid latitude: %s\n", argv[1]);
      return -1;
    }

    lon = mySrtToDouble ((const char *)argv[2]);
    if (myStrToDoubleError)
    {
      fprintf(stdout, "Invalid longitude: %s\n", argv[2]);
      return -1;
    }
    
    getLoc (result, CITY_NAME_LEN_MAX, lat, lon);
    //fprintf (stdout, "%f, %f = %s\n", lat, lon, result);
    fprintf (stdout, "%s\n", result);
  }

  if(koreaCityLocList)
  {
    free ((void *) koreaCityLocList);
    koreaCityLocList = NULL;
  }

  return 0;

}

/* member functions */
void getLoc(char *result, int maxResultSize, double lat, double lon)
{
	return azim_dist(result, maxResultSize, lat, lon);
}


void azim_dist(char *result, int maxResultSize, double lat, double lon)
{
  #define INVALID_DISTANCE (9999999.0)
  int i;
  int nearestCity = -1;
  double nearestDist = INVALID_DISTANCE;
  double nearestAzim, nearestBazim;

  for (i = 0; i < nKoreaCities; ++i)
  {
    double dist, azim, bazim;

		mb_locdiff(koreaCityLocList[i].lat, koreaCityLocList[i].lon, lat, lon, &dist, &azim, &bazim);
    dist = (dist * BC_PI * 2 * 6371.028) / 360;
      
    if(dist < nearestDist)
    {
      nearestCity = i;
      nearestAzim = azim;
      nearestBazim = bazim;
      nearestDist = dist;
    }
  }

  if((nearestDist != INVALID_DISTANCE) && (nearestCity > 0))
  {
    char distString[16] = {0x00};

    nearestDist = floor(100 * (nearestDist + 0.005)) / 100;
    //sprintf(distString, "%.2f", nearestDist);
    sprintf(distString, "%.0f", nearestDist);

    strcpy(result, koreaCityLocList[nearestCity].name);
    strcat(result, " ");  

    if ((nearestAzim >= 0 && nearestAzim < 11.25) || (nearestAzim >= 348.75 && nearestAzim < 360))
    {
      strcat(result, "남");
    }
    else if (nearestAzim >= 11.25 && nearestAzim < 33.75)
    {
      strcat(result, "남남서");
    }
    else if (nearestAzim >= 33.75 && nearestAzim < 56.25)
    {
      strcat(result, "남서");
    }
    else if (nearestAzim >= 56.25 && nearestAzim < 78.75)
    {
      strcat(result, "서남서");
    }
    else if (nearestAzim >= 78.75 && nearestAzim < 101.25)
    {
      strcat(result, "서");
    }
    else if (nearestAzim >= 101.25 && nearestAzim < 123.75)
    {
      strcat(result, "서북서");
    }
    else if (nearestAzim >= 123.75 && nearestAzim < 146.25)
    {
      strcat(result, "북서");
    }
    else if (nearestAzim >= 146.25 && nearestAzim < 168.75)
    {
      strcat(result, "북북서");
    }
    else if (nearestAzim >= 168.75 && nearestAzim < 191.25)
    {
      strcat(result, "북");
    }
    else if (nearestAzim >= 191.25 && nearestAzim < 213.75)
    {
      strcat(result, "북북동");
    }
    else if (nearestAzim >= 213.75 && nearestAzim < 236.25)
    {
      strcat(result, "북동");
    }
    else if (nearestAzim >= 236.25 && nearestAzim < 258.75)
    {
      strcat(result, "동북동");
    }
    else if (nearestAzim >= 258.75 && nearestAzim < 281.25)
    {
      strcat(result, "동");
    }
    else if (nearestAzim >= 281.25 && nearestAzim < 303.75)
    {
      strcat(result, "동남동");
    }
    else if (nearestAzim >= 303.75 && nearestAzim < 326.25)
    {
      strcat(result, "남동");
    }
    else if (nearestAzim >= 326.25 && nearestAzim < 348.75)
    {
      strcat(result, "남남동");
    }
    else
    {
      fprintf (stdout, "Unexpected azimuth value: %f\n", nearestAzim);
    }
    
    strcat(result, "쪽 약 ");
    strcat(result, distString);
    strcat(result, "km");

  }

	return;
}
	

void mb_locdiff ( double slat, double slon, double elat, double elon, double *distance, double *azim, double *bazim )
/* returns distance and azimuth in degrees of two locations on earth
 *
 * parameters of routine
 * double     slat, slon;    input; latitude and longitude of station
 * double     elat, elon;    input; latitude and longitude of epicentre
 * double     *distance;     output; distance in degrees
 * double     *azim;         output; azimuth in degrees
 * double     *bazim;        output; back-azimuth in degrees
 */
{
  /* local variables */
    double   ass, bs, cs, ds;
    double   ae, be, ce, de;
    double   bls, cbls, sbls, ble;
    double   codel, bgdel;
    double   xi, xj, xk;
    double   sindt, cosz, sinz;

  /* executable code */
    mb_geocr(slon, slat, &ass, &bs, &cs);
    ds = sqrt(1.0 - cs * cs);
    mb_geocr(elon, elat, &ae, &be, &ce);
    de = sqrt(1.0 - ce * ce);

  bls = DEG2RAD( slon );
    cbls = cos( bls );
    sbls = sin(bls);
    codel = ae*ass + be*bs + ce*cs;

    sindt = sqrt(1.0 - codel * codel);
    if  (codel == 0.0)  {
            bgdel = BC_PI/2.0;
    } else {
        bgdel = atan(fabs(sindt / codel));
            if  (codel <= 0.0)
                    bgdel = BC_PI - bgdel;
    } /*endif*/

    *distance = RAD2DEG( bgdel );

  /* azimuths */
    xi = bs*ce - be*cs;
    xj = ass*ce - ae*cs;
    xk = ass*be - ae*bs;
    cosz = (xi*sbls + xj*cbls)/sindt;
    sinz = xk/(ds*sindt);
    *bazim = mb_azm( cosz, sinz );
    ble = DEG2RAD( elon );
    cosz = -(xi * sin(ble) + xj * cos(ble)) / sindt;
  sinz = -xk/(de*sindt);
    *azim = mb_azm( cosz, sinz );

} /* end of mb_locdiff */



void mb_geocr(double lon, double lat, double *a, double *b, double *c)
{
    double blbda, bphi, ep, ug, vg;

    /* executable code */
    blbda = DEG2RAD(lon);
    bphi = DEG2RAD(lat);
    ep = 1.0 - 1.0 / 297.0;
    ug = ep * ep * tan(bphi);
    vg = 1.0 / sqrt(1.0 + ug * ug);
    *a = vg * cos(blbda);
    *b = vg * sin(blbda);
    *c = ug * vg;

} /* end of mb_geocr */

double mb_azm(double x, double y)
{
    /* local variables */
    double th;

    /* executable code */
    if (x == 0.0)
    {
        if (y > 0.0) return 90.0;
        if (y < 0.0) return 270.0;
        return 0.0;
    } /*endif*/

    th = RAD2DEG(atan(fabs(y / x)));
    if (x > 0.0)
    {
        if (y < 0.0) return (360.0 - th);
        return th;
    }
    else
    {
        if (y >= 0.0) return (180.0 - th);
        return (180.0 + th);
    } /*endif*/

} /* end of mb_azm */


