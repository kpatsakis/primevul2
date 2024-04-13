static void DumpDLS (S_EAS *pEAS)
{
    S_DLS_ARTICULATION *pArt;
    S_DLS_REGION *pRegion;
    EAS_INT i;
    EAS_INT j;

    EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000022 , pEAS->numPrograms);
    EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000023 , pEAS->numWTRegions);
    EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000024 , pEAS->numDLSArticulations);
    EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000025 , pEAS->numSamples);

 /* dump the instruments */
 for (i = 0; i < pEAS->numPrograms; i++)
 {
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000026 ,
                pEAS->pPrograms[i].locale >> 16,
 (pEAS->pPrograms[i].locale >> 8) & 0x7f,
                pEAS->pPrograms[i].locale & 0x7f);

 for (j = pEAS->pPrograms[i].regionIndex; ; j++)
 {
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000027 , j);
            pRegion = &pEAS->pWTRegions[j];
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000028 , pRegion->gain);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000029 , pRegion->region.rangeLow, pRegion->region.rangeHigh);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002a , pRegion->region.keyGroupAndFlags);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002b , pRegion->loopStart);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002c , pRegion->loopEnd);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002d , pRegion->tuning);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002e , pRegion->artIndex);
            EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000002f , pRegion->waveIndex);

 if (pRegion->region.keyGroupAndFlags & REGION_FLAG_LAST_REGION)
 break;
 }

 }

 /* dump the articulation data */
 for (i = 0; i < pEAS->numDLSArticulations; i++)
 {
 /* articulation data */
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000030 , i);
        pArt = &pEAS->pDLSArticulations[i];
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000031 , pArt->m_nEG2toFilterDepth);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000032 , pArt->m_nEG2toPitchDepth);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000033 , pArt->m_nFilterCutoffFrequency);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000034 , pArt->m_nFilterResonance);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000035 , pArt->m_nLFOAmplitudeDepth);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000036 , pArt->m_nLFODelayTime);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000037 , pArt->m_nLFOFrequency);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000038 , pArt->m_nLFOPitchDepth);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000039 , pArt->m_nPan);

 /* EG1 data */
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003a , pArt->m_sEG1.m_nAttack);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003b , pArt->m_sEG1.m_nDecay);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003c , pArt->m_sEG1.m_nSustain);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003d , pArt->m_sEG1.m_nRelease);

 /* EG2 data */
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003e , pArt->m_sEG2.m_nAttack);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x0000003f , pArt->m_sEG2.m_nDecay);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000040 , pArt->m_sEG2.m_nSustain);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000041 , pArt->m_sEG2.m_nRelease);

 }

 /* dump the waves */
 for (i = 0; i < pEAS->numSamples; i++)
 {
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000042 , i);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000043 , pEAS->pSampleLen[i]);
        EAS_ReportEx(_EAS_SEVERITY_NOFILTER, 0x19299ed4, 0x00000044 , pEAS->ppSamples[i]);
 }

}
