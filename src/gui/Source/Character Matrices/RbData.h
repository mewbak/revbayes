#import <Cocoa/Cocoa.h>
@class RbDataCell;
@class RbTaxonData;



@interface RbData : NSObject <NSCoding> {

	NSMutableArray*      data;
    int                  dataType;
    BOOL                 isHomologyEstablished;
    NSString*            alignmentMethod;
	NSString*            name;
	int                  numCharacters;
	int                  numTaxa;
	NSMutableArray*      taxonNames;
	NSMutableSet*        excludedTaxa;
	NSMutableSet*        excludedCharacters;
    RbData*              copiedFrom;
}

@property (readwrite)        int       dataType;
@property (readwrite)        BOOL      isHomologyEstablished;
@property (readwrite,retain) NSString* name;
@property (readwrite,retain) NSString* alignmentMethod;
@property (readwrite)        int       numCharacters;
@property (readwrite)        int       numTaxa;

- (void)addTaxonData:(RbTaxonData*)td;
- (void)addTaxonName:(NSString*)n;
- (RbDataCell*)cellWithRow:(int)r andColumn:(int)c;
- (void)cleanName:(NSString*)nameStr;
- (void)clear;
- (RbData*)copiedFrom;
- (int)dataSize;
- (void)deleteLastTaxon;
- (void)excludeTaxonIndexed:(int)idx;
- (void)excludeCharacterIndexed:(int)idx;
- (RbTaxonData*)getDataForTaxonIndexed:(int)idx;
- (RbTaxonData*)getDataForTaxonWithName:(NSString*)ns;
- (void)includeAllCharacters;
- (void)includeAllTaxa;
- (int)indexOfTaxonNamed:(NSString*)nme;
- (id)initWithRbData:(RbData*)d;
- (BOOL)isCharacterExcluded:(int)idx;
- (BOOL)isTaxonExcluded:(int)idx;
- (BOOL)isCharacterMissAmbig:(int)idx;
- (BOOL)isTaxonNamePresent:(NSString*)theName;
- (int)maxNumCharacters;
- (int)numExcludedCharacters;
- (int)numExcludedTaxa;
- (int)numCharactersForTaxon:(int)idx;
- (void)print;
- (void)restoreTaxonIndexed:(int)idx;
- (void)restoreCharacterIndexed:(int)idx;
- (void)setCopiedFrom:(RbData*)d;
- (void)setNameOfTaxonWithIndex:(int)idx to:(NSString*)newName;
- (void)setStandardMatrixToHave:(int)rows andToHave:(int)columns;
- (void)writeToFile:(NSString*)fn;
- (NSString*)taxonWithIndex:(int)i;

@end
