SVGImage::~SVGImage()
{
    if (m_page) {
        OwnPtr<Page> currentPage = m_page.release();
        currentPage->mainFrame()->loader().frameDetached(); // Break both the loader and view references to the frame
    }

    ASSERT(!m_chromeClient || !m_chromeClient->image());
}
