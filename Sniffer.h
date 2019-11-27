class Sniffer
{
  public:
    Sniffer();
    static bool isEqual(int A, int B, int tolerance);
    bool available();
    int nPulse();
    int GetPauseLenght();
    void continueReceive();
    void disableReceive();
    int GetStartPulseLenght();
    int GetMinPulseLenght();
    int GetMaxPulseLenght();
    unsigned int GetCode(int index);
  private:
    static void SignalInterrupt();
};
