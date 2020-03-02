/*****************************************
Kaid Sadikot
1054779
2020/02/23

I have used code from the starter files provided in class,
as well as code from: http://www.xmlsoft.org/examples/tree2.c
for my xmltree creation and saving xml to file.

*****************************************/

#include "SVGParser.h"
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#define M_PI 3.14159265358979323846

//Additional helper function definitions
void initLists(SVGimage **img);
void iterateTree(xmlNode *startNode, SVGimage **imageStruct);
void createTitle(xmlNode *elemNode, SVGimage **imageStruct);
void createDescription(xmlNode *elemNode, SVGimage **imageStruct);
void svgAttr(xmlNode *elemNode, SVGimage **imageStruct);
void getNameSpace(xmlNode *elemNode, SVGimage **imageStruct);
Attribute* createAttrStruct(char *name, char *value);
Group* createGroup(xmlNode *elemNode);
Rectangle* createRect(xmlNode *elemNode);
Circle* createCircle(xmlNode *elemNode);
Path* createPath(xmlNode *elemNode);
void setRects(List *list, List *path);
void setCircs(List *list, List *path);
void setGroups(List *list, List *path);
void setPaths(List *list, List *path);
int searchElement(List* list, bool (*customCompare)(void* first, void* second), void* searchElem);
bool recCompare(void* data, void* search);
bool circCompare(void* data, void* search);
bool pathCompare(void *data, void *search);
int lengthOfGroup(Group *grpPtr);
int getNumGroups(List *path, int len);
int attrCounter(List *path);
int groupAttr(List* path);
xmlDocPtr createXmlTree(SVGimage * img);
void dummyDelete(void *data);
void setRectstoXml(xmlNodePtr rootNode, List* recList);
bool schemaValidation(xmlDoc *doc, char *schemaFile);
void setCircstoXml(xmlNodePtr rootNode, List *cirList);
void setPathtoXml(xmlNodePtr rootNode, List *pathList);
void setGrouptoXml(xmlNodePtr rootNode, List *grpList);
bool SVGvaldation(SVGimage *doc);
bool validateRect(List *recList);
bool validateAttr(List *attrList);
bool validateCirc(List *cirList);
bool validatePath(List *pthList);
bool validateGroup(List *grpList);
void setSVGattr(SVGimage *image, Attribute *newAttribute);
void setRectAttr(SVGimage *image, int index, Attribute *newAttribute);
void setCircAttr(SVGimage *image, int index, Attribute *newAttribute);
void setPathAttr(SVGimage *image, int index, Attribute *newAttribute);
void setGroupAttr(SVGimage *image, int index, Attribute *newAttribute);

SVGimage* createSVGimage(char* fileName)
{
  xmlDoc *doc = NULL;
  xmlNode *node = NULL;
  SVGimage *imagePtr = malloc(sizeof(SVGimage));

  imagePtr->title[0] = '\0';
  imagePtr->description[0] = '\0';
  imagePtr->namespace[0] = '\0';

  doc = xmlReadFile(fileName, NULL, 0);

  if (doc == NULL)
  {
    printf("Could not parse file\n");
    return NULL;
  }

  node = xmlDocGetRootElement(doc);

  initLists(&imagePtr);

  iterateTree(node, &imagePtr);

  xmlFreeDoc(doc);

  xmlCleanupParser();

  return imagePtr;

}

char* SVGimageToString(SVGimage* img)
{

  char *imgStr = NULL;
  char *recStr = NULL;
  char *circStr = NULL;
  char *attrStr = NULL;
  char *pathStr = NULL;
  char *grpStr = NULL;
  int len = 0;

  if (img == NULL)
    return NULL;

  if (getFromFront(img->rectangles) != NULL) {
    recStr = img->rectangles->printData(getFromFront(img->rectangles));
    len = len + strlen(recStr);
  }

  if (getFromFront(img->circles) != NULL) {
    circStr = img->circles->printData(getFromFront(img->circles));
    len = len + strlen(circStr);
  }

  if (getFromFront(img->paths) != NULL) {
    pathStr = img->paths->printData(getFromFront(img->paths));
    len = len + strlen(pathStr);
  }

  if (getFromFront(img->otherAttributes) != NULL) {
    attrStr = img->otherAttributes->printData(getFromFront(img->otherAttributes));
    len = len + strlen(attrStr);
  }

  if (getFromFront(img->groups) != NULL) {
    grpStr = img->groups->printData(getFromFront(img->groups));
    len = len + strlen(grpStr);
  }

  imgStr = malloc(sizeof(char) * len + 12);

  sprintf(imgStr, "SVG Image: ");

  if (recStr)
    imgStr = strcat(imgStr, recStr);

  if (circStr)
    imgStr = strcat(imgStr, circStr);

  if (pathStr)
    imgStr = strcat(imgStr, pathStr);

  if (attrStr)
    imgStr = strcat(imgStr, attrStr);

  if (grpStr)
    imgStr = strcat(imgStr, grpStr);

  free(recStr);
  free(circStr);
  free(pathStr);
  free(attrStr);
  free(grpStr);

  return imgStr;

}

void deleteSVGimage(SVGimage* img)
{

  if (img == NULL)
    return;

  freeList(img->rectangles);
  freeList(img->groups);
  freeList(img->circles);
  freeList(img->paths);
  freeList(img->otherAttributes);
  free(img);

}

// Function that returns a list of all rectangles in the image.
List* getRects(SVGimage* img)
{

  if (img == NULL)
    return NULL;

  List *recList = initializeList(&rectangleToString,&dummyDelete, &compareRectangles);
  ListIterator iter = createIterator(img->rectangles);
  void *elem;

  while ((elem = nextElement(&iter)) != NULL) {
    Rectangle *recPtr = elem;

    insertBack(recList, recPtr);
  }

  setRects(recList, img->groups);

  return recList;

}

// Function that returns a list of all circles in the image.
List* getCircles(SVGimage* img)
{
  if (img == NULL)
    return NULL;

  List *circList = initializeList(&circleToString,&dummyDelete, &compareCircles);
  ListIterator iter = createIterator(img->circles);
  void *elem;


  while ((elem = nextElement(&iter)) != NULL) {
    Circle *circPtr = elem;
    insertBack(circList, circPtr);
  }

  setCircs(circList, img->groups);

  return circList;
}

// Function that returns a list of all groups in the image.
List* getGroups(SVGimage* img)
{
  if (img == NULL)
    return NULL;

  List *grpList = initializeList(&groupToString,&dummyDelete, &compareGroups);
  ListIterator iter = createIterator(img->groups);
  void *elem;

  while ((elem = nextElement(&iter)) != NULL) {
    Group *grpPtr = elem;
    insertBack(grpList, grpPtr);

    if (getFromFront(grpPtr->groups) != NULL) {
      setGroups(grpList, grpPtr->groups);
    }
  }

  return grpList;
}
// Function that returns a list of all paths in the image.
List* getPaths(SVGimage* img)
{
  if (img == NULL)
    return NULL;

  List *pathList = initializeList(&pathToString,&dummyDelete, &comparePaths);
  ListIterator iter = createIterator(img->paths);
  void *elem;

  while ((elem = nextElement(&iter)) != NULL) {
    Path *pathPtr = elem;

    insertBack(pathList, pathPtr);
  }

  setPaths(pathList, img->groups);

  return pathList;
}

void dummyDelete(void *data)
{
  return;
}

void setRects(List *list, List *path)
{
  ListIterator grpiter = createIterator(path);
  void *elem;

  while ((elem = nextElement(&grpiter)) != NULL) {
    Group *grpPtr = elem;
    ListIterator recIter = createIterator(grpPtr->rectangles);
    void *recElem;

    while ((recElem = nextElement(&recIter)) != NULL) {
      Rectangle *recPtr = recElem;
      insertBack(list, recPtr);

    }

    if (getFromFront(grpPtr->groups) != NULL) {
      setRects(list, grpPtr->groups);
    }
  }

}


void setCircs(List *list, List *path)
{
  ListIterator grpiter = createIterator(path);
  void *elem;

  while ((elem = nextElement(&grpiter)) != NULL) {
    Group *grpPtr = elem;
    ListIterator cirIter = createIterator(grpPtr->circles);
    void *cirElem;
    while ((cirElem = nextElement(&cirIter)) != NULL) {
      Circle *cirPtr = cirElem;
      insertFront(list, cirPtr);

    }

    if (getFromFront(grpPtr->groups) != NULL) {
      setCircs(list, grpPtr->groups);
    }
  }
}

void setGroups(List *list, List *path)
{
  ListIterator grpiter = createIterator(path);
  void *elem;

  while ((elem = nextElement(&grpiter)) != NULL) {
    Group *grpPtr = elem;
    insertBack(list, grpPtr);

    if (getFromFront(grpPtr->groups) != NULL) {
      setGroups(list, grpPtr->groups);
    }
  }
}

void setPaths(List *list, List *path)
{
  ListIterator grpiter = createIterator(path);
  void *elem;

  while ((elem = nextElement(&grpiter)) != NULL) {
    Group *grpPtr = elem;
    ListIterator pathIter = createIterator(grpPtr->paths);
    void *pathElem;
    while ((pathElem = nextElement(&pathIter)) != NULL) {
      Path *pathPtr =pathElem;
      insertFront(list, pathPtr);

    }

    if (getFromFront(grpPtr->groups) != NULL) {
      setPaths(list, grpPtr->groups);
    }
  }
}

// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(SVGimage* img, float area)
{
  if (img == NULL || area == 0)
  {
    return 0;
  }

  List *list = getRects(img);

  int count = 0;
  count = searchElement(list, &recCompare, &area);

  freeList(list);
  return count;

}

bool recCompare(void* data, void* search)
{
  if(data == NULL)
    return false;

  Rectangle *recPtr = data;
  float find = *((float *)search);
  float area = ceil(recPtr->width * recPtr->height);

  if (area == ceil(find)) {
    return true;
  } else {
    return false;
  }
}


// Function that returns the number of all circles with the specified area
int numCirclesWithArea(SVGimage* img, float area)
{
  if (img == NULL)
    return 0;

  List *list = getCircles(img);

  int count = 0;
  count = searchElement(list, &circCompare, &area);

  freeList(list);

  return count;

}

bool circCompare(void* data, void* search)
{
  if (data == NULL)
    return false;

  Circle *cirPtr = data;
  float find = *((float *)search);
  float area = ceil(M_PI * pow(cirPtr->r, 2));

  if (area == ceil(find)) {
    return true;
  } else {
    return false;
  }
}

// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(SVGimage* img, char* data)
{

  if (img == NULL){
    return 0;
  }

  int count = 0;
  List *list = getPaths(img);

  count = searchElement(list, &pathCompare, data);
  freeList(list);

  return count;

}

bool pathCompare(void *data, void *search)
{
  if (data == NULL)
    return false;

  Path *pathPtr = data;
  char *searchData = search;

  if (strcmp(pathPtr->data, searchData) == 0)
  {
    return true;
  } else {
    return false;
  }
}
// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(SVGimage* img, int len)
{
  if (img == NULL)
    return 0;

  int count = 0;
  int grpLen = 0;

  ListIterator itr = createIterator(img->groups);

  Group *data = nextElement(&itr);
  while (data != NULL)
  {
    grpLen = lengthOfGroup(data);

    if (grpLen == len)
      count++;

    if (getFromFront(data->groups) != NULL){
      count = count + getNumGroups(data->groups, len);
    }

    data = nextElement(&itr);
  }

  return count;
}

int getNumGroups(List *path, int len)
{
  int count = 0;
  int grpLen = 0;
  ListIterator itr = createIterator(path);
  Group *data = nextElement(&itr);

  while (data != NULL)
  {
    grpLen = lengthOfGroup(data);

    if (grpLen == len)
      count++;

    if (getFromFront(data->groups) != NULL){
      count = count + getNumGroups(data->groups, len);
    }

    data = nextElement(&itr);
  }
  return count;
}

int lengthOfGroup(Group *grpPtr)
{
  int count = 0;

  count += getLength(grpPtr->rectangles);
  count += getLength(grpPtr->circles);
  count += getLength(grpPtr->paths);
  count += getLength(grpPtr->groups);

  return count;
}

int numAttr(SVGimage* img)
{
  if (img == NULL)
    return 0;

  int count = 0;
  ListIterator rectItr = createIterator(img->rectangles);
  ListIterator circItr = createIterator(img->circles);
  ListIterator pathItr = createIterator(img->paths);

  count = attrCounter(img->otherAttributes);

  Rectangle *rectData = nextElement(&rectItr);
  while (rectData != NULL)
  {
    count = count + attrCounter(rectData->otherAttributes);
    rectData = nextElement(&rectItr);
  }

  Circle *circData = nextElement(&circItr);
  while (circData != NULL)
  {
    count = count + attrCounter(circData->otherAttributes);
    circData = nextElement(&circItr);
  }

  Path *pathData = nextElement(&pathItr);
  while (pathData != NULL)
  {
    count = count + attrCounter(pathData->otherAttributes);
    pathData = nextElement(&pathItr);
  }

  count = count + groupAttr(img->groups);

  return count;

}

int groupElementsAttr(Group *grp)
{

  int count = 0;
  ListIterator rectItr = createIterator(grp->rectangles);
  ListIterator circItr = createIterator(grp->circles);
  ListIterator pathItr = createIterator(grp->paths);

  Rectangle *rectData = nextElement(&rectItr);
  while (rectData != NULL)
  {
    count = count + attrCounter(rectData->otherAttributes);
    rectData = nextElement(&rectItr);
  }

  Circle *circData = nextElement(&circItr);
  while (circData != NULL)
  {
    count = count + attrCounter(circData->otherAttributes);
    circData = nextElement(&circItr);
  }

  Path *pathData = nextElement(&pathItr);
  while (pathData != NULL)
  {
    count = count + attrCounter(pathData->otherAttributes);
    pathData = nextElement(&pathItr);
  }

  return count;
}

int groupAttr(List* path)
{
  int count = 0;
  ListIterator itr = createIterator(path);

  Group *grpData = nextElement(&itr);
  while (grpData != NULL)
  {
    count = count + attrCounter(grpData->otherAttributes);
    count = count + groupElementsAttr(grpData);

    if (getFromFront(grpData->groups) != NULL) {
      count = count + groupAttr(grpData->groups);
    }

    grpData = nextElement(&itr);
  }

  return count;
}

int attrCounter(List *path)
{
  int count = 0;
  ListIterator itr = createIterator(path);

  Attribute * data = nextElement(&itr);
  while (data != NULL)
  {
    count++;
    data= nextElement(&itr);
  }

  return count;

}

int searchElement(List* list, bool (*customCompare)(void* first, void* second), void* searchElem)
{
  if (customCompare == NULL)
    return 0;

  int count = 0;
  ListIterator itr = createIterator(list);

  void* data = nextElement(&itr);
  while (data != NULL)
  {
    if(customCompare(data, searchElem))

      count++;

    data = nextElement(&itr);
  }

  return count;

}


/***************************Helper Functions************************/

void deleteAttribute( void* data)
{
  Attribute *tmpAttr;

  if (data == NULL)
    return;

  tmpAttr = (Attribute*)data;

  free(tmpAttr->name);
  free(tmpAttr->value);
  free(tmpAttr);
}

char* attributeToString( void* data)
{
  char* tmpStr;
  Attribute *tmpAttr;
  int len;

  if (data == NULL)
    return NULL;

  tmpAttr = (Attribute*)data;
  len = strlen(tmpAttr->name) + strlen(tmpAttr->value) + 23;
  tmpStr = (char*)malloc(sizeof(char) * len);

  sprintf(tmpStr, "Attr[Name: %s value: %s]", tmpAttr->name, tmpAttr->value);

  return tmpStr;

}

int compareAttributes(const void *first, const void *second)
{
  return 0;

}

void deleteGroup(void* data)
{
  Group *tmpGrp;

  if (data == NULL)
    return;

  tmpGrp = data;

  freeList(tmpGrp->rectangles);
  freeList(tmpGrp->circles);
  freeList(tmpGrp->paths);
  freeList(tmpGrp->groups);
  freeList(tmpGrp->otherAttributes);
  free(tmpGrp);
}

char* groupToString( void* data)
{
  char *tmpStr = NULL;
  char *recStr = NULL;
  char *circStr = NULL;
  char *attrStr = NULL;
  char *pathStr = NULL;
  char *grpStr = NULL;
  Group *tmpGrp = NULL;
  int len = 0;

  if (data == NULL)
    return NULL;

  tmpGrp = data;


  if (getFromFront(tmpGrp->rectangles) != NULL) {
    recStr = tmpGrp->rectangles->printData(getFromFront(tmpGrp->rectangles));
    len = len + strlen(recStr);
  }

  if (getFromFront(tmpGrp->circles) != NULL) {
    circStr = tmpGrp->circles->printData(getFromFront(tmpGrp->circles));
    len = len + strlen(circStr);
  }

  if (getFromFront(tmpGrp->paths) != NULL) {
    pathStr = tmpGrp->paths->printData(getFromFront(tmpGrp->paths));
    len = len + strlen(pathStr);
  }

  if (getFromFront(tmpGrp->otherAttributes) != NULL) {
    attrStr = tmpGrp->otherAttributes->printData(getFromFront(tmpGrp->otherAttributes));
    len = len + strlen(attrStr);
  }

  if (getFromFront(tmpGrp->groups) != NULL) {
    grpStr = tmpGrp->groups->printData(getFromFront(tmpGrp->groups));
    len = len + strlen(grpStr);
  }

  tmpStr = malloc(sizeof(char) * len + 10);

  sprintf(tmpStr,"Group[");

  if (recStr)
    tmpStr = strcat(tmpStr, recStr);

  if (circStr)
    tmpStr = strcat(tmpStr, circStr);

  if (pathStr)
    tmpStr = strcat(tmpStr, pathStr);

  if (attrStr)
    tmpStr = strcat(tmpStr, attrStr);

  if (grpStr)
    tmpStr = strcat(tmpStr, grpStr);

  tmpStr = strcat(tmpStr, "]");

  free(recStr);
  free(circStr);
  free(pathStr);
  free(attrStr);
  free(grpStr);

  return tmpStr;

}

int compareGroups(const void *first, const void *second)
{
  return 0;
}

void deleteRectangle(void* data)
{
  Rectangle *tmpRec;

  if (data == NULL)
    return;

  tmpRec = data;

  freeList(tmpRec->otherAttributes);
  free(tmpRec);
}

char* rectangleToString(void* data)
{
  char *tmpStr;
  Rectangle *tmpRec;
  int len;

  if (data == NULL)
    return NULL;

  tmpRec = data;
  len = 111;
  tmpStr = malloc(sizeof(char) *len);

  sprintf(tmpStr,"Rect[x: %.2f y: %.2f width: %.2f height: %.2f units: %s]", tmpRec->x, tmpRec->y, tmpRec->width, tmpRec->height, tmpRec->units);

  return tmpStr;
}

int compareRectangles(const void *first, const void *second)
{
  return 0;
}

void deleteCircle(void* data)
{
  Circle *tmpCir;

  if (data == NULL)
    return;

  tmpCir = (Circle*)data;

  freeList(tmpCir->otherAttributes);
  free(tmpCir);
}

char* circleToString(void* data)
{
  char *tmpStr;
  Circle *tmpCir;
  int len = 83;

  if (data == NULL)
    return NULL;

  tmpCir = data;
  tmpStr = malloc(sizeof(char) * len);

  sprintf(tmpStr, "Circ[cx: %.2f cy: %.2f r: %.2f units: %s]", tmpCir->cx, tmpCir->cy, tmpCir->r, tmpCir->units);

  return tmpStr;

}

int compareCircles(const void *first, const void *second)
{
  return 0;
}

void deletePath(void* data)
{
  Path *tmpPath;

  if (data == NULL)
    return;

  tmpPath = (Path*)data;

  free(tmpPath->data);
  freeList(tmpPath->otherAttributes);
  free(tmpPath);
}

char* pathToString(void* data)
{
  char *tmpStr;
  Path *tmpPath;
  int len;

  if (data == NULL)
    return NULL;

  tmpPath = data;
  len = strlen(tmpPath->data) + 10;
  tmpStr = malloc(sizeof(char) * len);

  sprintf(tmpStr, "Path[ %s]", tmpPath->data);

  return tmpStr;

}

int comparePaths(const void *first, const void *second)
{
  return 0;
}


/*******************Additional Helper Functions*******************************/

void initLists(SVGimage **img)
{
  (*img)->groups = initializeList(&groupToString,&deleteGroup, &compareGroups);
  (*img)->paths = initializeList(&pathToString,&deletePath, &comparePaths);
  (*img)->circles = initializeList(&circleToString,&deleteCircle, &compareCircles);
  (*img)->rectangles = initializeList(&rectangleToString,&deleteRectangle, &compareRectangles);
  (*img)->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
}

void iterateTree(xmlNode *startNode, SVGimage **imageStruct)
{
  xmlNode *currNode = NULL;
  Rectangle *tmpRec;
  Path *tmpPath;
  Circle *tmpCir;
  Group *tmpGrp;

  for(currNode = startNode; currNode != NULL;currNode = currNode->next)
  {
    if (strcmp((char*)(currNode->name), "svg") == 0) {
      svgAttr(currNode, imageStruct);
      getNameSpace(currNode, imageStruct);
    }

    if (strcmp((char*)(currNode->name), "title") == 0)
      createTitle(currNode, imageStruct);

    if (strcmp((char*)(currNode->name), "desc") == 0)
      createDescription(currNode, imageStruct);

    if (strcmp((char*)(currNode->name), "rect") == 0) {
      tmpRec = createRect(currNode);
      insertBack((*imageStruct)->rectangles, tmpRec);
    }

    if (strcmp((char*)(currNode->name), "path") == 0) {
      tmpPath = createPath(currNode);
      insertBack((*imageStruct)->paths, tmpPath);
    }

    if (strcmp((char*)(currNode->name), "circle") == 0) {
      tmpCir = createCircle(currNode);
      insertBack((*imageStruct)->circles, tmpCir);
    }

    if (strcmp((char*)(currNode->name), "g") == 0) {
      tmpGrp = createGroup(currNode);
      insertBack((*imageStruct)->groups, tmpGrp);
    }
    else {
      iterateTree(currNode->children, imageStruct);
    }
  }
}

Group* createGroup(xmlNode *elemNode)
{
  Group *grpStruct;
  Rectangle *subRec;
  Circle *subCirc;
  Path *subPath;
  Group *subGrp;
  xmlNode *childNode;
  char *nodeName;

  Attribute *otherAttr;
  xmlAttr *attr;
  xmlNode *value;
  char *attrName;
  char *cont;

  grpStruct = malloc(sizeof(Group));

  grpStruct->groups = initializeList(&groupToString,&deleteGroup, &compareGroups);
  grpStruct->paths = initializeList(&pathToString,&deletePath, &comparePaths);
  grpStruct->circles = initializeList(&circleToString,&deleteCircle, &compareCircles);
  grpStruct->rectangles = initializeList(&rectangleToString,&deleteRectangle, &compareRectangles);
  grpStruct->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

  for(childNode = elemNode->children; childNode != NULL; childNode = childNode->next)
  {
    nodeName = (char*)childNode->name;

    if (strcasecmp(nodeName, "rect") == 0) {
      subRec = createRect(childNode);
      insertBack(grpStruct->rectangles, subRec);

    } else if (strcasecmp(nodeName, "circle") == 0) {
      subCirc = createCircle(childNode);
      insertBack(grpStruct->circles, subCirc);

    } else if (strcasecmp(nodeName, "path") == 0) {
      subPath = createPath(childNode);
      insertBack(grpStruct->paths, subPath);

    } else if (strcasecmp(nodeName, "g") == 0) {
      subGrp = createGroup(childNode);
      insertBack(grpStruct->groups, subGrp);

    }
  }

  for(attr = elemNode->properties; attr !=NULL; attr = attr->next)
  {
    value = attr->children;
    attrName = (char *)attr->name;
    cont = (char*)value->content;

    otherAttr = createAttrStruct(attrName, cont);
    insertBack(grpStruct->otherAttributes, otherAttr);
  }

  return grpStruct;
}

Rectangle* createRect(xmlNode *elemNode)
{
  xmlAttr *attr;
  xmlNode *value;
  float num;
  char *attrName;
  char *cont;
  Rectangle *rectStruct = NULL;
  Attribute *otherAttr;

  rectStruct = malloc(sizeof(Rectangle));
  List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  rectStruct->x = 0;
  rectStruct->y = 0;
  rectStruct->width = 0;
  rectStruct->height = 0;
  rectStruct->units[0] = '\0';

  for(attr = elemNode->properties; attr !=NULL; attr = attr->next)
  {
    value = attr->children;
    attrName = (char *)attr->name;
    cont = (char*)value->content;

    if (strcasecmp(attrName, "x") == 0) {
      num = atof(cont);
      rectStruct->x = num;

    } else if (strcasecmp(attrName, "y") == 0) {
      num = atof(cont);
      rectStruct->y = num;

    } else if (strcasecmp(attrName, "width") == 0) {

      num = atof(cont);
      rectStruct->width = num;

      int i = 0;
      do {

        if (isalpha(*cont) != 0) {
          rectStruct->units[i] = *cont;
          i++;
        }

        rectStruct->units[i] = '\0';

        cont++;
      } while (*cont != '\0');

    } else if (strcasecmp(attrName, "height") == 0) {
      num = atof(cont);
      rectStruct->height = num;

    } else {
      otherAttr = createAttrStruct(attrName, cont);
      insertBack(attrList, otherAttr);

    }
  }

  rectStruct->otherAttributes = attrList;

  return rectStruct;

}

Circle* createCircle(xmlNode *elemNode)
{
  xmlAttr *attr;
  xmlNode *value;
  float num;
  char *attrName;
  char *cont;
  Circle *circStruct;
  Attribute *otherAttr;

  circStruct = malloc(sizeof(Circle));
  List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  circStruct->cx = 0;
  circStruct->cy = 0;
  circStruct->r = 0;
  circStruct->units[0] = '\0';
  for(attr = elemNode->properties; attr !=NULL; attr = attr->next)
  {
    value = attr->children;
    attrName = (char *)attr->name;
    cont = (char*)value->content;

    if (strcasecmp(attrName, "cx") == 0) {
      num = atof(cont);
      circStruct->cx = num;

    } else if (strcasecmp(attrName, "cy") == 0) {
      num = atof(cont);
      circStruct->cy = num;

    } else if (strcasecmp(attrName, "r") == 0) {
      num = atof(cont);
      circStruct->r = num;

      int i = 0;
      do {

        if (isalpha(*cont) != 0) {
          circStruct->units[i] = *cont;
          i++;
        }

        circStruct->units[i] = '\0';

        cont++;
      } while (*cont != '\0');

    } else {
      otherAttr = createAttrStruct(attrName, cont);
      insertBack(attrList, otherAttr);

    }
  }

  circStruct->otherAttributes = attrList;
  return circStruct;
}

Path* createPath(xmlNode *elemNode)
{
  xmlAttr *attr;
  xmlNode *value;
  char *attrName;
  char *cont;
  Path *pathStruct;
  Attribute *otherAttr;

  pathStruct = malloc(sizeof(Path));
  List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  pathStruct->data = NULL;

  for(attr = elemNode->properties; attr !=NULL; attr = attr->next)
  {
    value = attr->children;
    attrName = (char*)attr->name;
    cont = (char*)value->content;

    if (strcasecmp(attrName, "d") == 0) {
      pathStruct->data = malloc((sizeof(char) * strlen(cont)) + 1);
      strcpy(pathStruct->data, cont);

    } else {
      otherAttr = createAttrStruct(attrName, cont);
      insertBack(attrList, otherAttr);
    }

  }

  pathStruct->otherAttributes = attrList;
  return pathStruct;

}

void svgAttr(xmlNode *elemNode, SVGimage **imageStruct)
{
  xmlAttr* attr;
  xmlNode *value;
  char *attrName;
  char *cont;
  Attribute* attrStruct;


  for (attr = elemNode->properties; attr != NULL; attr = attr->next)
  {
    value = attr->children;
    attrName = (char *)attr->name;
    cont = (char *)(value->content);
    attrStruct = createAttrStruct(attrName, cont);

    insertBack((*imageStruct)->otherAttributes, attrStruct);

  }

}

Attribute* createAttrStruct(char *name, char *value)
{
  Attribute* attrStruct = malloc(sizeof(Attribute));

  attrStruct->name = malloc((sizeof(char) * strlen(name)) + 1);
  attrStruct->value = malloc((sizeof(char) * strlen(value)) + 1);

  strcpy(attrStruct->name, name);
  strcpy(attrStruct->value, value);

  return attrStruct;

}

void getNameSpace(xmlNode *elemNode, SVGimage **imageStruct)
{
  xmlNs *nsStruct = elemNode->ns;
  char *nameSpace = (char*)(nsStruct->href);

  strncpy((char*)((*imageStruct)->namespace), nameSpace, 256);
}

void createTitle(xmlNode *elemNode, SVGimage **imageStruct)
{
  char *cont;

  if (elemNode->children->content != NULL)
  {
    cont = (char*)(elemNode->children->content);
    memset((*imageStruct)->title,'\0', sizeof((*imageStruct)->title));
    strncpy((*imageStruct)->title, cont, 256);

  }

}

void createDescription(xmlNode *elemNode, SVGimage **imageStruct)
{
  char *cont;

  if (elemNode->children->content != NULL)
  {
    cont = (char*)(elemNode->children->content);
    memset((*imageStruct)->description, '\0', sizeof((*imageStruct)->description));
    strncat((*imageStruct)->description, cont, 256);
  }
}

/****************************A2 Functions***************************/
/********************************MOD 1******************************/
SVGimage* createValidSVGimage(char* fileName, char* schemaFile)
{

  if (fileName == NULL || fileName[0] == '\0' || schemaFile == NULL || schemaFile[0] == '\0' || strstr(fileName, ".svg") == NULL || strstr(schemaFile, ".xsd") == NULL){
    return NULL;
  }

  SVGimage *imagePtr;
  xmlDoc *doc;

  doc = xmlReadFile(fileName, NULL, 0);

  if (schemaValidation(doc, schemaFile) == false)
  {
    return NULL;
  } else {
    imagePtr = createSVGimage(fileName);
  }


  xmlFreeDoc(doc);
  return imagePtr;

}

bool writeSVGimage(SVGimage* image, char* fileName)
{

  if (image == NULL || fileName == NULL || fileName[0] == '\0' || strstr(fileName, ".svg") == NULL)
    return false;

  xmlDocPtr xmlPtr = createXmlTree(image);
  if (xmlSaveFormatFileEnc(fileName, xmlPtr, NULL, 1) != -1){
    xmlFreeDoc(xmlPtr);
    xmlCleanupParser();
    return true;
  } else {
    return false;
  }
}

bool validateSVGimage(SVGimage* doc, char* schemaFile)
{
  if (doc == NULL || schemaFile == NULL || schemaFile[0] == '\0') {
    return false;
  }

  bool schemaValid = NULL;
  bool imageValid = NULL;

  if ((imageValid = SVGvaldation(doc)) == false) {
    return imageValid;
  }

  xmlDocPtr xmlPtr = createXmlTree(doc);

  if (xmlPtr == NULL) {

    xmlCleanupParser();
    return false;
  } else {

    if ((schemaValid = schemaValidation(xmlPtr, schemaFile))){
      xmlFreeDoc(xmlPtr);
      xmlCleanupParser();
    } else {
      return false;
    }
  }

  if (imageValid == true && schemaValid == true)
  {
    return true;
  } else {
    return false;
  }

}

/*******************************MOD 2********************************/
void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute)
{
  if (image == NULL || newAttribute == NULL || newAttribute->name == NULL || newAttribute->value == NULL)
    return;

  if (elemType == SVG_IMAGE) {
    setSVGattr(image, newAttribute);
  } else if (elemType == RECT) {
    setRectAttr(image, elemIndex, newAttribute);
  } else if (elemType == CIRC) {
    setCircAttr(image, elemIndex, newAttribute);
  } else if (elemType == PATH) {
    setPathAttr(image, elemIndex, newAttribute);
  } else if (elemType == GROUP) {
    setGroupAttr(image, elemIndex, newAttribute);
  }
}

void addComponent(SVGimage* image, elementType elemType, void* newComponent)
{
  if (image == NULL || newComponent == NULL)
    return;

  if (elemType == RECT) {
    Rectangle *newRect = newComponent;
    insertBack(image->rectangles,newRect);

  } else if (elemType == CIRC) {
    Circle *newCirc = newComponent;
    insertBack(image->circles,newCirc);

  } else if (elemType == PATH) {
    Path *newPath = newComponent;
    insertBack(image->paths,newPath);

  }
}


/*******************************MOD 3**********************************/

char* attrToJSON(const Attribute *a)
{
  if (a == NULL)
    return NULL;

  char *attrStr;

  if (a == NULL) {
    attrStr = malloc(sizeof(char) * 3);
    strcpy(attrStr, "{}");
    return attrStr;
  } else {
    attrStr = malloc(sizeof(char) * (22 + strlen(a->name) + strlen(a->value) + 1));
    sprintf(attrStr, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
    return attrStr;
  }
}

char* circleToJSON(const Circle *c)
{
  if (c == NULL)
    return NULL;
  char *circStr;

  if (c == NULL) {
    circStr = malloc(sizeof(char) *3);
    strcpy(circStr, "{}");
    return circStr;
  } else {
    circStr = malloc(sizeof(char) * (40 + 12 + 4 + strlen(c->units)) + 1);
    sprintf(circStr, "{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}",c->cx,c->cy,c->r,getLength(c->otherAttributes), c->units);
    return circStr;
  }
}

char* rectToJSON(const Rectangle *r)
{
  if (r == NULL)
    return NULL;
  char *rectStr;

  if (r == NULL) {
    rectStr = malloc(sizeof(char) *3);
    strcpy(rectStr, "{}");
    return rectStr;
  } else {
    rectStr = malloc(sizeof(char) * (45 + 16 + 4 + strlen(r->units)) + 1);
    sprintf(rectStr, "{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}",r->x,r->y,r->width,r->height,getLength(r->otherAttributes), r->units);
    return rectStr;
  }
}

char* pathToJSON(const Path *p)
{
  if (p == NULL)
    return NULL;
  char *pathStr;
  char data[65] = "\0";

  if (p == NULL) {
    pathStr = malloc(sizeof(char) *3);
    strcpy(pathStr, "{}");
    return pathStr;
  } else {
    strncpy(data,p->data,64);
    pathStr = malloc(sizeof(char) * (24 + strlen(data)));
    sprintf(pathStr, "{\"d\":\"%s\",\"numAttr\":%d}", data, getLength(p->otherAttributes));
    return pathStr;
  }
}

char* groupToJSON(const Group *g)
{
  if (g == NULL)
    return NULL;
  char *grpStr;

  if (g == NULL) {
    grpStr = malloc(sizeof(char) * 3);
    strcpy(grpStr, "{}");
    return grpStr;
  } else {
    grpStr = malloc(sizeof(char) *25 + 8);
    sprintf(grpStr, "{\"children\":%d,\"numAttr\":%d}", lengthOfGroup((Group*)g), getLength(g->otherAttributes));
    return grpStr;
  }
}

char* SVGtoJSON(const SVGimage* imge)
{
  if (imge == NULL)
    return NULL;
  char *svgStr;

  if (imge == NULL) {
    svgStr = malloc(sizeof(char) * 3);
    strcpy(svgStr, "{}");
    return svgStr;
  }
  List *recList = getRects((SVGimage*)imge);
  List *cirList = getCircles((SVGimage*)imge);
  List *pthList = getPaths((SVGimage*)imge);
  List *grpList = getGroups((SVGimage*)imge);

  svgStr = malloc(sizeof(char) * (49 + 16));
  sprintf(svgStr, "{\"numRect\":%d,\"numCirc\":%d,\"numPaths\":%d,\"numGroups\":%d}", getLength(recList), getLength(cirList), getLength(pthList), getLength(grpList));

  freeList(recList);
  freeList(cirList);
  freeList(pthList);
  freeList(grpList);

  return svgStr;
}

char* attrListToJSON(const List *list)
{
  char *bufStr;
  int bufSize = 0;

  if (list == NULL) {
    bufStr = malloc(sizeof(char) * 3);
    strcpy(bufStr,"[]");
    return bufStr;
  }

  ListIterator itr = createIterator((List*)list);
  Attribute *attr = NULL;
  Attribute *lastAttr = getFromBack((List*) list);
  char *tmpStr;
  bufStr = malloc(2);
  strcpy(bufStr, "[");

  while((attr = nextElement(&itr)) != NULL) {
    tmpStr = attrToJSON(attr);
    bufSize += strlen(tmpStr) + 1;
    bufStr = realloc(bufStr, (sizeof(char) * bufSize) + 2);
    strcat(bufStr,tmpStr);
    if (attr != lastAttr)
      strcat(bufStr,",");
    free(tmpStr);

  }

  bufStr = realloc(bufStr, bufSize + 3);
  strcat(bufStr, "]\0");

  return bufStr;

}

char* circListToJSON(const List *list)
{

  char *bufStr;
  int bufSize = 0;

  if (list == NULL) {
    bufStr = malloc(sizeof(char) * 3);
    strcpy(bufStr, "[]");
    return bufStr;
  }

  ListIterator itr = createIterator((List*) list);
  Circle *circ = NULL;
  Circle *lastCirc = getFromBack((List*)list);
  char *tmpStr;
  bufStr = malloc(2);
  strcpy(bufStr,"[");

  while ((circ = nextElement(&itr)) != NULL) {
    tmpStr = circleToJSON(circ);
    bufSize += strlen(tmpStr) + 1;
    bufStr = realloc(bufStr, (sizeof(char) * bufSize) + 2);
    strcat(bufStr,tmpStr);
    if (circ != lastCirc)
      strcat(bufStr,",");
    free(tmpStr);
  }

  bufStr = realloc(bufStr, bufSize + 3);
  strcat(bufStr,"]\0");

  return bufStr;

}

char* rectListToJSON(const List *list)
{
  char *bufStr;
  int bufSize = 0;

  if (list == NULL) {
    bufStr = malloc(sizeof(char) * 3);
    strcpy(bufStr, "[]");
    return bufStr;
  }

  ListIterator itr = createIterator((List*)list);
  Rectangle *rect = NULL;
  Rectangle *lastRect = getFromBack((List*)list);
  char *tmpStr;
  bufStr = malloc(2);
  strcpy(bufStr,"[");

  while ((rect = nextElement(&itr)) != NULL) {
    tmpStr = rectToJSON(rect);
    bufSize += strlen(tmpStr) + 1;
    bufStr = realloc(bufStr, (sizeof(char) * bufSize) + 2);
    strcat(bufStr, tmpStr);
    if (rect != lastRect)
      strcat(bufStr, ",");
    free(tmpStr);
  }

  bufStr = realloc(bufStr, bufSize + 3);
  strcat(bufStr, "]\0");

  return bufStr;
}

char* pathListToJSON(const List *list)
{
  char *bufStr;
  int bufSize = 0;

  if (list == NULL) {
    bufStr = malloc(3);
    strcpy(bufStr, "[]");
    return bufStr;
  }

  ListIterator itr = createIterator((List*)list);
  Path *path = NULL;
  Path *lastPth = getFromBack((List*)list);
  char *tmpStr;
  bufStr = malloc(2);
  strcpy(bufStr, "[");

  while ((path = nextElement(&itr)) != NULL) {
    tmpStr = pathToJSON(path);
    bufSize += strlen(tmpStr) + 1;
    bufStr = realloc(bufStr, (sizeof(char) * bufSize) + 2);
    strcat(bufStr, tmpStr);
    if (path != lastPth)
      strcat(bufStr, ",");
    free(tmpStr);
  }

  bufStr = realloc(bufStr, bufSize + 3);
  strcat(bufStr, "]\0");

  return bufStr;
}

char* groupListToJSON(const List *list)
{
  char *bufStr;
  int bufSize = 0;

  if (list == NULL) {
    bufStr = malloc(sizeof(char) * 3);
    strcpy(bufStr, "[]");
    return bufStr;
  }

  ListIterator itr = createIterator((List*)list);
  Group *group = NULL;
  Group *lastgrp = getFromBack((List*)list);
  char *tmpStr;
  bufStr = malloc(2);
  strcpy(bufStr,"[");

  while ((group = nextElement(&itr)) != NULL) {
    tmpStr = groupToJSON(group);
    bufSize += strlen(tmpStr) + 1;
    bufStr = realloc(bufStr, (sizeof(char) * bufSize) + 2);
    strcat(bufStr, tmpStr);
    if (group != lastgrp)
      strcat(bufStr, ",");
    free(tmpStr);
  }

  bufStr = realloc(bufStr, bufSize + 3);
  strcat(bufStr, "]\0");

  return bufStr;
}

/**************************Bonus MOD 3*********************************/
SVGimage* JSONtoSVG(const char* svgString) {
  return NULL;
}

Rectangle* JSONtoRect(const char* svgString) {
  return NULL;
}

Circle* JSONtoCircle(const char* svgString) {
  return NULL;
}
/**************************A2 Helper Functions*************************/

void setSVGattr(SVGimage *image, Attribute *newAttribute)
{

  ListIterator iter = createIterator(image->otherAttributes);
  Attribute *svgAttr = NULL;

  while ((svgAttr = nextElement(&iter))) {
    if (strcmp(newAttribute->name, svgAttr->name) == 0) {
      free(svgAttr->value);
      svgAttr->value = malloc((sizeof(char) * strlen(newAttribute->value)) + 1);
      strcpy(svgAttr->value, newAttribute->value);

      free(newAttribute->name);
      free(newAttribute->value);
      free(newAttribute);
      return;
    }
  }

  insertBack(image->otherAttributes, newAttribute);
}

void setRectAttr(SVGimage *image, int index, Attribute *newAttribute)
{

  ListIterator rectItr = createIterator(image->rectangles);
  Rectangle *myRect = NULL;
  float num;

  if (getLength(image->rectangles) < (index + 1) || index < 0)
    return;

  for (int i = 0; i < index + 1; i++) {
    myRect = nextElement(&rectItr);
  }

  if (strcmp(newAttribute->name, "x") == 0) {
    num = atof(newAttribute->value);
    myRect->x = num;

  } else if (strcmp(newAttribute->name, "y") == 0) {
    num = atof(newAttribute->value);
    myRect->y = num;

  } else if (strcmp(newAttribute->name, "width") == 0) {
    num = atof(newAttribute->value);
    myRect->width = num;

  } else if (strcmp(newAttribute->name, "height") == 0) {
    num = atof(newAttribute->value);
    myRect->height = num;

  } else {

    ListIterator iter = createIterator(myRect->otherAttributes);
    Attribute *myAttr = NULL;

    while ((myAttr = nextElement(&iter))) {
      if (strcmp(newAttribute->name, myAttr->name) == 0) {
        free(myAttr->value);
        myAttr->value = malloc((sizeof(char) * strlen(newAttribute->value)) + 1);
        strcpy(myAttr->value, newAttribute->value);

        free(newAttribute->name);
        free(newAttribute->value);
        free(newAttribute);
        return;
      }
    }
    insertBack(myRect->otherAttributes, newAttribute);
    return;


  }
  free(newAttribute->name);
  free(newAttribute->value);
  free(newAttribute);
}

void setCircAttr(SVGimage *image, int index, Attribute *newAttribute)
{

  ListIterator circItr = createIterator(image->circles);
  Circle *myCirc = NULL;
  float num;

  if (getLength(image->circles) < (index + 1) || index < 0) {
    return;
  }

  for (int i = 0; i < index + 1; i++) {
    myCirc = nextElement(&circItr);
  }

  if (strcmp(newAttribute->name, "cx") == 0) {
    num = atof(newAttribute->value);
    myCirc->cx = num;

  } else if (strcmp(newAttribute->name, "cy") == 0) {
    num = atof(newAttribute->value);
    myCirc->cy = num;

  } else if (strcmp(newAttribute->name, "r") == 0) {
    num = atof(newAttribute->value);
    myCirc->r = num;

  } else {
    ListIterator iter = createIterator(myCirc->otherAttributes);
    Attribute *myAttr = NULL;

    while ((myAttr = nextElement(&iter))) {
      if (strcmp(newAttribute->name, myAttr->name) == 0) {
        free(myAttr->value);
        myAttr->value = malloc((sizeof(char) * strlen(newAttribute->value)) + 1);
        strcpy(myAttr->value, newAttribute->value);

        free(newAttribute->name);
        free(newAttribute->value);
        free(newAttribute);
        return;
      }
    }

    insertBack(myCirc->otherAttributes, newAttribute);
    return;

  }
  free(newAttribute->name);
  free(newAttribute->value);
  free(newAttribute);
}

void setPathAttr(SVGimage *image, int index, Attribute *newAttribute)
{

  ListIterator pthItr = createIterator(image->paths);
  Path *myPath = NULL;

  if (getLength(image->paths) < (index + 1) || index < 0)
    return;

  for (int i = 0; i < index + 1; i++) {
    myPath = nextElement(&pthItr);
  }

  if (strcmp(newAttribute->name, "d") == 0) {
    free(myPath->data);
    myPath->data = malloc(sizeof(char) * strlen(newAttribute->value) + 1);
    strcpy(myPath->data, newAttribute->value);

  } else {
    ListIterator iter = createIterator(myPath->otherAttributes);
    Attribute *myAttr = NULL;

    while ((myAttr = nextElement(&iter))) {
      if (strcmp(newAttribute->name, myAttr->name) == 0) {
        free(myAttr->value);
        myAttr->value = malloc((sizeof(char) * strlen(newAttribute->value)) + 1);
        strcpy(myAttr->value, newAttribute->value);

        free(newAttribute->name);
        free(newAttribute->value);
        free(newAttribute);
        return;
      }
    }

    insertBack(myPath->otherAttributes, newAttribute);
    return;

  }
  free(newAttribute->name);
  free(newAttribute->value);
  free(newAttribute);
}

void setGroupAttr(SVGimage *image, int index, Attribute *newAttribute)
{


  ListIterator grpItr = createIterator(image->groups);
  Group *myGrp = NULL;

  for (int i = 0; i < index + 1; i++) {
    myGrp = nextElement(&grpItr);
  }

  ListIterator iter = createIterator(myGrp->otherAttributes);
  Attribute *myAttr = NULL;

  while ((myAttr = nextElement(&iter))) {
    if (strcmp(newAttribute->name, myAttr->name) == 0) {
      free(myAttr->value);
      myAttr->value = malloc((sizeof(char) * strlen(newAttribute->value)) + 1);
      strcpy(myAttr->value, newAttribute->value);

      free(newAttribute->name);
      free(newAttribute->value);
      free(newAttribute);
      return;
    }
  }
  insertBack(myGrp->otherAttributes, newAttribute);
  return;
}

bool schemaValidation(xmlDoc *doc, char *schemaFile)
{

  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;

  xmlLineNumbersDefault(1);

  ctxt = xmlSchemaNewParserCtxt(schemaFile);
  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);

  if (doc == NULL)
  {

    return false;
  }
  else
  {
    xmlSchemaValidCtxtPtr ctxt;
    int ret;

    ctxt = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxt, doc);
    if (ret != 0)
    {
      return false;
    } else {

      xmlSchemaFreeValidCtxt(ctxt);
      // free the resource
      if(schema != NULL)
        xmlSchemaFree(schema);

      xmlSchemaCleanupTypes();
      xmlCleanupParser();
      xmlMemoryDump();
      return true;
    }
  }
}

xmlDocPtr createXmlTree(SVGimage *img)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr rootNode = NULL;
  Attribute *svgAttr;

  doc = xmlNewDoc(BAD_CAST "1.0");
  rootNode = xmlNewNode(NULL, BAD_CAST "svg");
  xmlDocSetRootElement(doc, rootNode);

  ListIterator attrIter = createIterator(img->otherAttributes);
  while ((svgAttr = nextElement(&attrIter)) != NULL) {
    xmlNewProp(rootNode, BAD_CAST svgAttr->name, BAD_CAST svgAttr->value);
  }

  xmlNsPtr ns = xmlNewNs(rootNode, BAD_CAST img->namespace, NULL);
  xmlSetNs(rootNode, ns);

  if (img->description[0] != '\0') {
    xmlNewChild(rootNode, NULL, BAD_CAST "desc", BAD_CAST img->description);
  }

  if (img->title[0] != '\0') {
    xmlNewChild(rootNode, NULL, BAD_CAST "title", BAD_CAST img->title);
  }

  setRectstoXml(rootNode, img->rectangles);
  setCircstoXml(rootNode, img->circles);
  setPathtoXml(rootNode, img->paths);
  setGrouptoXml(rootNode, img->groups);

  return doc;

}

void setRectstoXml(xmlNodePtr rootNode, List *recList)
{

  xmlNodePtr recNode;
  ListIterator recIter = createIterator(recList);
  Rectangle *recPtr = NULL;
  Attribute *otherAttr = NULL;
  char xBuf[20] = "\0";
  char yBuf[20] = "\0";
  char wBuf[20] = "\0";
  char hBuf[20] = "\0";

  while ((recPtr = nextElement(&recIter)) != NULL) {

    sprintf(xBuf, "%f", recPtr->x);
    sprintf(yBuf, "%f", recPtr->y);
    sprintf(wBuf, "%f", recPtr->width);
    sprintf(hBuf, "%f", recPtr->height);

    if (recPtr->units[0] != '\0') {
      strcat(xBuf, recPtr->units);
      strcat(yBuf, recPtr->units);
      strcat(wBuf, recPtr->units);
      strcat(hBuf, recPtr->units);
    }

    recNode = xmlNewChild(rootNode, NULL, BAD_CAST "rect", NULL);
    xmlNewProp(recNode, BAD_CAST "x", BAD_CAST xBuf);
    xmlNewProp(recNode, BAD_CAST "y", BAD_CAST yBuf);
    xmlNewProp(recNode,BAD_CAST "width",BAD_CAST wBuf);
    xmlNewProp(recNode, BAD_CAST "height", BAD_CAST hBuf);

    ListIterator attrIter = createIterator(recPtr->otherAttributes);
    while ((otherAttr = nextElement(&attrIter)) != NULL) {
      xmlNewProp(recNode, BAD_CAST otherAttr->name, BAD_CAST otherAttr->value);
    }
  }

}

void setCircstoXml(xmlNodePtr rootNode, List *cirList)
{
  xmlNodePtr cirNode;
  ListIterator cirIter = createIterator(cirList);
  Circle *cirPtr = NULL;
  Attribute *otherAttr = NULL;
  char cxBuf[20] = "\0";
  char cyBuf[20] = "\0";
  char rBuf[20] = "\0";

  while ((cirPtr = nextElement(&cirIter)) != NULL) {

    sprintf(cxBuf, "%f", cirPtr->cx);
    sprintf(cyBuf, "%f", cirPtr->cy);
    sprintf(rBuf, "%f", cirPtr->r);

    if (cirPtr->units[0] != '\0') {
      strcat(rBuf,cirPtr->units);
    }

    cirNode = xmlNewChild(rootNode, NULL, BAD_CAST "circle", NULL);
    xmlNewProp(cirNode, BAD_CAST "cx", BAD_CAST cxBuf);
    xmlNewProp(cirNode, BAD_CAST "cy", BAD_CAST cyBuf);
    xmlNewProp(cirNode, BAD_CAST "r", BAD_CAST rBuf);

    ListIterator attrIter = createIterator(cirPtr->otherAttributes);
    while ((otherAttr = nextElement(&attrIter)) != NULL) {
      xmlNewProp(cirNode, BAD_CAST otherAttr->name, BAD_CAST otherAttr->value);
    }

  }
}

void setPathtoXml(xmlNodePtr rootNode, List *pathList)
{
  xmlNodePtr pthNode;
  ListIterator pthIter = createIterator(pathList);
  Path *pthPtr = NULL;
  Attribute *otherAttr = NULL;

  while ((pthPtr = nextElement(&pthIter))) {
    pthNode = xmlNewChild(rootNode, NULL, BAD_CAST "path", NULL);

    ListIterator attrIter = createIterator(pthPtr->otherAttributes);
    while ((otherAttr = nextElement(&attrIter)) != NULL) {
      xmlNewProp(pthNode, BAD_CAST otherAttr->name, BAD_CAST otherAttr->value);
    }
    xmlNewProp(pthNode, BAD_CAST "d", BAD_CAST pthPtr->data);
  }
}

void setGrouptoXml(xmlNodePtr rootNode, List *grpList)
{
  xmlNodePtr grpNode;
  ListIterator grpIter = createIterator(grpList);
  Group *grpPtr = NULL;
  Attribute *otherAttr = NULL;

  while((grpPtr = nextElement(&grpIter))) {
    grpNode = xmlNewChild(rootNode, NULL, BAD_CAST "g", NULL);

    ListIterator attrIter = createIterator(grpPtr->otherAttributes);
    while ((otherAttr = nextElement(&attrIter)) != NULL) {
      xmlNewProp(grpNode, BAD_CAST otherAttr->name, BAD_CAST otherAttr->value);
    }

    setRectstoXml(grpNode, grpPtr->rectangles);
    setCircstoXml(grpNode, grpPtr->circles);
    setPathtoXml(grpNode, grpPtr->paths);
    setGrouptoXml(grpNode, grpPtr->groups);
  }
}

bool SVGvaldation(SVGimage *doc)
{

  if(doc->namespace == NULL || doc->namespace[0] == '\0')
    return false;

  if(doc->title == NULL || doc->description == NULL)
    return false;

  if (doc->rectangles == NULL || doc->circles == NULL || doc->paths == NULL || doc->groups == NULL || doc->otherAttributes == NULL)
    return false;

  if(!(validateAttr(doc->otherAttributes))) {
    return false;
  }

  if (!(validateRect(doc->rectangles))) {
    return false;
  }

  if (!(validateCirc(doc->circles))) {
    return false;
  }
  if (!(validatePath(doc->paths))) {
    return false;
  }

  if (!(validateGroup(doc->groups))) {
    return false;
  }

  return true;
}

bool validateAttr(List *attrList)
{
  ListIterator iter = createIterator(attrList);
  Attribute *attr = NULL;

  while ((attr = nextElement(&iter))) {
    if (attr->name == NULL || attr->value == NULL) {
      return false;
    }
  }
  return true;
}

bool validateRect(List *recList)
{
  ListIterator iter = createIterator(recList);
  Rectangle *recPtr = NULL;

  while ((recPtr = nextElement(&iter))) {

    if (recPtr->width < 0 || recPtr->height < 0)
      return false;

    if (recPtr->otherAttributes == NULL)
      return false;

    if (recPtr->units[0] != '\0') {
        if (strcmp(recPtr->units, "em") != 0 && strcmp(recPtr->units, "ex") != 0 && strcmp(recPtr->units, "px") != 0 && strcmp(recPtr->units, "pt") != 0 && strcmp(recPtr->units, "pc") != 0 && strcmp(recPtr->units, "cm") != 0 && strcmp(recPtr->units, "mm") != 0 && strcmp(recPtr->units, "in") != 0)
          return false;
    }
    if (!(validateAttr(recPtr->otherAttributes)))
      return false;
  }
  return true;
}

bool validateCirc(List *cirList)
{
  ListIterator iter = createIterator(cirList);
  Circle *cirPtr = NULL;

  while((cirPtr = nextElement(&iter))) {
    if (cirPtr->r < 0)
      return false;

    if (cirPtr->otherAttributes == NULL)
      return false;

    if (cirPtr->units[0] != '\0') {
        if (strcmp(cirPtr->units, "em") != 0 && strcmp(cirPtr->units, "ex") != 0 && strcmp(cirPtr->units, "px") != 0 && strcmp(cirPtr->units, "pt") != 0 && strcmp(cirPtr->units, "pc") != 0 && strcmp(cirPtr->units, "cm") != 0 && strcmp(cirPtr->units, "mm") != 0 && strcmp(cirPtr->units, "in") != 0)
          return false;
    }

    if (!(validateAttr(cirPtr->otherAttributes)))
      return false;
  }
  return true;
}

bool validatePath(List *pthList)
{
  ListIterator iter = createIterator(pthList);
  Path *pthPtr = NULL;

  while((pthPtr = nextElement(&iter))) {
    if (pthPtr->data == NULL)
      return false;

    if (pthPtr->otherAttributes == NULL)
      return false;

    if (!(validateAttr(pthPtr->otherAttributes)))
      return false;
  }

  return true;
}

bool validateGroup(List *grpList)
{
  ListIterator iter = createIterator(grpList);
  Group *grpPtr = NULL;

  while((grpPtr = nextElement(&iter))) {
    if (grpPtr->rectangles == NULL || grpPtr->circles == NULL || grpPtr->paths == NULL ||grpPtr->groups == NULL)
      return false;

    if(!(validateAttr(grpPtr->otherAttributes)))
      return false;

    if (!(validateRect(grpPtr->rectangles)))
      return false;

    if (!(validateCirc(grpPtr->circles)))
      return false;

    if (!(validatePath(grpPtr->paths)))
      return false;

    if (!(validateGroup(grpPtr->groups)))
      return false;
  }
  return true;
}
