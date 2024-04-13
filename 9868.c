    ~EncryptingStream() {
        m_Output->Release();
        delete m_StreamCipher;
    }