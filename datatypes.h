//#pragma pack(1)
typedef struct buttonData {
   uint8_t id;
   Rect_t area;
   char *text;
} __attribute__ ((packed)) ButtonData;

//#pragma pack(1)
typedef struct listBoxData {
   uint8_t id;
   Rect_t area;
   char *text;
   char elements[10][10];
} __attribute__ ((packed)) ListBoxData;
