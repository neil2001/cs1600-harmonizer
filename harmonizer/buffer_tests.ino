bool testWriteBuffer() {
  int buf[4] = {0};
  int idx = 2;
  int val = 1;
  insertToBuffer(val, idx, buf, 4);
  return idx == 3 && buf[2] == 1;
}

bool testWriteBufferEnd() {
  int buf[4] = {0};
  int idx = 3;
  int val = 1;
  insertToBuffer(val, idx, buf, 4);
  return idx == 0 && buf[3] == 1;
}

bool testReadBuffer() {
  int buf[4] = {1,2,3,4};
  int idx = 0;
  int val = readFromBuffer(idx, buf, 4);
  return idx == 1 && val == 1;
}

bool testReadBufferEnd() {
  int buf[4] = {1,2,3,4};
  int idx = 3;
  int val = readFromBuffer(idx, buf, 4);
  return idx == 0 && val == 4;
}

bool testWritesFast() {
  writecount = 0;
  unsigned long startTime = micros();
  while (writecount < 1000);
  unsigned long endTime = micros();

  int elapsedTimeMillis = (int) (endTime - startTime) / 1000;

  return elapsedTimeMillis < 50;
}

bool testReadsFast() {
  readcount = 0;
  unsigned long startTime = micros();
  while (readcount < 1000) {
    readADCSync();
    readcount++;
  }
  unsigned long endTime = micros();

  int elapsedTimeMillis = (int) (endTime - startTime) / 1000;

  return elapsedTimeMillis < 50;
}

bool runTest(TestFunctionPtr test, int n) {
  bool res = test();
  if (res) {
    Serial.print("Test ");
    Serial.print(n);
    Serial.println(" passed");
  } else {
    Serial.print("Test ");
    Serial.print(n);
    Serial.println(" failed");
  }
  return res;
}

bool runBufferTests() {
  bool t1 = runTest(testWriteBuffer, 1);
  bool t2 = runTest(testWriteBufferEnd, 2);
  bool t3 = runTest(testReadBuffer, 3);
  bool t4 = runTest(testReadBufferEnd, 4);
  bool t5 = runTest(testWritesFast, 5);
  bool t6 = runTest(testReadsFast, 6);

  bool all = t1 && t2 && t3 && t4 && t5 && t6;
  if (all) {
    Serial.println("All tests passed!");
  }
}
