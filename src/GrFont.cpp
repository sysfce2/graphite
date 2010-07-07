#include "GrFont.h"

GrFont::GrFont(float ppm, const LoadedFace *face) :
    m_face(face),
    m_scale(ppm / face->upem())
{
    size_t nGlyphs=m_face->numGlyphs();
    m_advances = new float[nGlyphs];
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}


/*virtual*/ GrFont::~GrFont()
{
    delete[] m_advances;
}


/*virtual*/ float GrFont::computeAdvance(unsigned short glyphid) const
{
    return m_face->getAdvance(glyphid, m_scale);
}



GrHintedFont::GrHintedFont(const IFont *font/*not NULL*/, const LoadedFace *face) :
    GrFont(font->ppm(), face), 
    m_font(font)
{
}


/*virtual*/ float GrHintedFont::computeAdvance(unsigned short glyphid) const
{
    return m_font->advance(glyphid);
}



