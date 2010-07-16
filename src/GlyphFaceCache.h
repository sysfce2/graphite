#pragma once

#include "GlyphFace.h"
#include "graphiteng/IFace.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Segment;
class Features;


class GlyphFaceCacheHeader
{
public:
    virtual bool initialize(const IFace* iFace/*not NULL*/);    //return result indicates success. Do not use if failed.
    unsigned short numGlyphs() const { return m_nGlyphs; }
    unsigned short numAttrs() const { return m_numAttrs; }

private:
friend class GrFace;
friend class GlyphFace;
    const void* m_pGloc;
    size_t m_lLoca;     const void* m_pLoca;
    const void* m_pHead;
    const void* m_pGlyf;
    size_t m_lHmtx;     const void* m_pHmtx;
    const void* m_pHHea;
    const void* m_pGlat;

    unsigned short m_numAttrs;      // number of glyph attributes per glyph
    bool m_locFlagsUse32Bit;
    unsigned short m_nGlyphsWithGraphics;       //i.e. boundary box and advance
    unsigned short m_nGlyphsWithAttributes;
    unsigned short m_nGlyphs;                   // number of glyphs in the font. Max of the above 2.
};

class GlyphFaceCache : public GlyphFaceCacheHeader
{
public:
    static EGlyphCacheStrategy nearestSupportedStrategy(EGlyphCacheStrategy requested);
    static GlyphFaceCache* makeCache(const GlyphFaceCacheHeader& hdr, EGlyphCacheStrategy requested);

    GlyphFaceCache(const GlyphFaceCacheHeader& hdr) : GlyphFaceCacheHeader(hdr), m_nAccesses(0), m_nLoads(0) {}
    virtual ~GlyphFaceCache() {}
    
    virtual EGlyphCacheStrategy getEnum() const = 0 ;
    virtual const GlyphFace *glyph(unsigned short glyphid) const = 0 ;      //result may be changed by subsequent call with a different glyphid
    const GlyphFace *glyphSafe(unsigned short glyphid) const { return glyphid<numGlyphs()?glyph(glyphid):NULL; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { if (gattr>=numAttrs()) return 0; const GlyphFace*p=glyphSafe(gid); return p?p->getAttr(gattr):0; }

    CLASS_NEW_DELETE
    
protected:
    void incAccesses() const { ++m_nAccesses; }      //don't count an access as a change
    void incLoads() const { ++m_nLoads; }            //const to allow lazy loading
    
private:
friend class IFace;
    mutable unsigned long m_nAccesses;
    mutable unsigned long m_nLoads;
};


class GlyphFaceCacheOneItem : public GlyphFaceCache
{
public:
    void * operator new (size_t s, const GlyphFaceCacheHeader& /*hdr*/)
    {
        return malloc(s);
    }

    GlyphFaceCacheOneItem(const GlyphFaceCacheHeader& hdr);   //always use with the above new, passing in the same GlyphFaceCacheHeader
    virtual ~GlyphFaceCacheOneItem();

    virtual EGlyphCacheStrategy getEnum() const;
    virtual const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    
private:
    GlyphFace *glyphDirect() const { return (GlyphFace *)&m_Buffer[0];}

private:
    mutable unsigned int m_LoadedGlyphNo;   //-1 means none loaded
//    mutable GlyphFace m_Buffer;           //Not good - d'tor invoked twice
//    mutable char m_Buffer[sizeof(GlyphFace)];     //Not good possibly bad alignment on Solaris and similar
    mutable int m_Buffer[(sizeof(GlyphFace)+sizeof(int)-1)/sizeof(int)];
};


class GlyphFaceCacheLoadedOnDemand : public GlyphFaceCache
{
public:
    void * operator new (size_t s, const GlyphFaceCacheHeader& hdr)
    {
        return malloc(s + sizeof(GlyphFace*)*hdr.numGlyphs());
    }

    GlyphFaceCacheLoadedOnDemand(const GlyphFaceCacheHeader& hdr);   //always use with the above new, passing in the same GlyphFaceCacheHeader
    virtual ~GlyphFaceCacheLoadedOnDemand();

    virtual EGlyphCacheStrategy getEnum() const;
    virtual const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    
private:
    GlyphFace **glyphPtrDirect(unsigned short glyphid) const { return (GlyphFace **)((const char*)(this)+sizeof(GlyphFaceCacheLoadedOnDemand)+sizeof(GlyphFace*)*glyphid);}

private:      //defensive
    GlyphFaceCacheLoadedOnDemand(const GlyphFaceCacheLoadedOnDemand&);
    GlyphFaceCacheLoadedOnDemand& operator=(const GlyphFaceCacheLoadedOnDemand&);
};


class GlyphFaceCachePreloaded : public GlyphFaceCache
{
public:
    void * operator new (size_t s, const GlyphFaceCacheHeader& hdr)
    {
        return malloc(s + sizeof(GlyphFace)*hdr.numGlyphs());
    }

    GlyphFaceCachePreloaded(const GlyphFaceCacheHeader& hdr);   //always use with the above new, passing in the same GlyphFaceCacheHeader
    virtual ~GlyphFaceCachePreloaded();

    virtual EGlyphCacheStrategy getEnum() const;
    virtual const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    
private:
    const GlyphFace *glyphDirect(unsigned short glyphid) const { return (const GlyphFace *)((const char*)(this)+sizeof(GlyphFaceCachePreloaded)+sizeof(GlyphFace)*glyphid);}
    GlyphFace *glyphDirect(unsigned short glyphid) { return (GlyphFace *)((char*)(this)+sizeof(GlyphFaceCachePreloaded)+sizeof(GlyphFace)*glyphid);}

private:      //defensive
    GlyphFaceCachePreloaded(const GlyphFaceCachePreloaded&);
    GlyphFaceCachePreloaded& operator=(const GlyphFaceCachePreloaded&);
};


}}}} // namespace
