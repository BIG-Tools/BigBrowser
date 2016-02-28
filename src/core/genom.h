#ifndef GENOM_H
#define GENOM_H
#include <QtCore>
#include "sequence.h"
namespace big {
class Genom
{
public:
    Genom(const QString& filename);

    /*!
     * \brief chromosomCount
     * \return how many chromosom are in the genom
     */
    int chromosomCount();

    /*!
     * \brief chromosomLength
     * \param chromosom
     * \return the length of chromosom
     */
    int chromosomLength(const QString& chromosom);


    /*!
     * \brief sequence
     *
     * Warning : Genom should be indexed in faidx.
     * \see hasIndex
     * \param chromosom
     * \param pos
     * \param length
     * \return a dna sequence according param location
     */
    Sequence sequence(const QString& chromosom, qint64 pos, qint64 length);

    /*!
     * \brief name
     * \return the genom name
     */
    const QString& name() const;
    const QString& filename() const;

    /*!
     * \brief hasIndex
     *
     * Check if faidx exists for the genom
     * \return True if it exists. Otherwise return false
     */
    bool hasIndex();

    /*!
     * \brief createIndex
     *
     * Create the faidx for the genom. Il may takes some time
     * \todo Threading the process ?
     */
    void createIndex();


private:
    QString mFilename;
    QString mName;


};
}
#endif // GENOM_H