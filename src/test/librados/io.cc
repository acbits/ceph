// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*
// vim: ts=8 sw=2 smarttab

#include "include/rados/librados.h"
#include "include/rados/librados.hpp"
#include "test/librados/test.h"
#include "test/librados/TestCase.h"

#include <errno.h>
#include "gtest/gtest.h"

using namespace librados;
using std::string;

typedef RadosTest LibRadosIo;
typedef RadosTestEC LibRadosIoEC;
typedef RadosTestPP LibRadosIoPP;
typedef RadosTestECPP LibRadosIoECPP;

TEST_F(LibRadosIo, SimpleWrite) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  rados_ioctx_set_namespace(ioctx, "nspace");
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
}

TEST_F(LibRadosIoPP, SimpleWritePP) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
  ioctx.set_namespace("nspace");
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
}

TEST_F(LibRadosIoPP, ReadOpPP) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));

  {
      bufferlist op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl, op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist op_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl, op_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl1, read_bl2, op_bl;
      int rval1 = 1000, rval2 = 1002;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl1, &rval1);
      op.read(0, sizeof(buf), &read_bl2, &rval2);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl1.length());
      ASSERT_EQ(sizeof(buf), read_bl2.length());
      ASSERT_EQ(sizeof(buf) * 2, op_bl.length());
      ASSERT_EQ(0, rval1);
      ASSERT_EQ(0, rval2);
      ASSERT_EQ(0, memcmp(read_bl1.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl2.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(op_bl.c_str() + sizeof(buf), buf, sizeof(buf)));
  }

  {
      bufferlist op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(0, rval);
  }

  {
      bufferlist read_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl1, read_bl2;
      int rval1 = 1000, rval2 = 1002;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl1, &rval1);
      op.read(0, sizeof(buf), &read_bl2, &rval2);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl1.length());
      ASSERT_EQ(sizeof(buf), read_bl2.length());
      ASSERT_EQ(0, rval1);
      ASSERT_EQ(0, rval2);
      ASSERT_EQ(0, memcmp(read_bl1.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl2.c_str(), buf, sizeof(buf)));
  }
}

TEST_F(LibRadosIo, RoundTrip) {
  char buf[128];
  char buf2[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  memset(buf2, 0, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
  ASSERT_EQ(0, memcmp(buf, buf2, sizeof(buf)));
}

TEST_F(LibRadosIoPP, RoundTripPP) {
  char buf[128];
  char buf2[128];
  Rados cluster;
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
  bufferlist cl;
  ASSERT_EQ((int)sizeof(buf2), ioctx.read("foo", cl, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(buf, cl.c_str(), sizeof(buf)));
}

TEST_F(LibRadosIo, OverlappingWriteRoundTrip) {
  char buf[128];
  char buf2[64];
  char buf3[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), rados_write(ioctx, "foo", buf2, sizeof(buf2), 0));
  memset(buf3, 0xdd, sizeof(buf3));
  ASSERT_EQ((int)sizeof(buf3), rados_read(ioctx, "foo", buf3, sizeof(buf3), 0));
  ASSERT_EQ(0, memcmp(buf3, buf2, sizeof(buf2)));
  ASSERT_EQ(0, memcmp(buf3 + sizeof(buf2), buf, sizeof(buf) - sizeof(buf2)));
}

TEST_F(LibRadosIoPP, OverlappingWriteRoundTripPP) {
  char buf[128];
  char buf2[64];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl1, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  bufferlist bl2;
  bl2.append(buf2, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), ioctx.write("foo", bl2, sizeof(buf2), 0));
  bufferlist bl3;
  ASSERT_EQ((int)sizeof(buf), ioctx.read("foo", bl3, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(bl3.c_str(), buf2, sizeof(buf2)));
  ASSERT_EQ(0, memcmp(bl3.c_str() + sizeof(buf2), buf, sizeof(buf) - sizeof(buf2)));
}

TEST_F(LibRadosIo, WriteFullRoundTrip) {
  char buf[128];
  char buf2[64];
  char buf3[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  ASSERT_EQ(0, rados_write_full(ioctx, "foo", buf2, sizeof(buf2)));
  memset(buf3, 0xdd, sizeof(buf3));
  ASSERT_EQ((int)sizeof(buf2), rados_read(ioctx, "foo", buf3, sizeof(buf3), 0));
  ASSERT_EQ(0, memcmp(buf2, buf2, sizeof(buf2)));
}

TEST_F(LibRadosIoPP, WriteFullRoundTripPP) {
  char buf[128];
  char buf2[64];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl1, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  bufferlist bl2;
  bl2.append(buf2, sizeof(buf2));
  ASSERT_EQ(0, ioctx.write_full("foo", bl2));
  bufferlist bl3;
  ASSERT_EQ((int)sizeof(buf2), ioctx.read("foo", bl3, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(bl3.c_str(), buf2, sizeof(buf2)));
}

TEST_F(LibRadosIo, AppendRoundTrip) {
  char buf[64];
  char buf2[64];
  char buf3[sizeof(buf) + sizeof(buf2)];
  memset(buf, 0xde, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  memset(buf2, 0xad, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), rados_append(ioctx, "foo", buf2, sizeof(buf2)));
  memset(buf3, 0, sizeof(buf3));
  ASSERT_EQ((int)sizeof(buf3), rados_read(ioctx, "foo", buf3, sizeof(buf3), 0));
  ASSERT_EQ(0, memcmp(buf3, buf, sizeof(buf)));
  ASSERT_EQ(0, memcmp(buf3 + sizeof(buf), buf2, sizeof(buf2)));
}

TEST_F(LibRadosIoPP, AppendRoundTripPP) {
  char buf[64];
  char buf2[64];
  memset(buf, 0xde, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  memset(buf2, 0xad, sizeof(buf2));
  bufferlist bl2;
  bl2.append(buf2, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), ioctx.append("foo", bl2, sizeof(buf2)));
  bufferlist bl3;
  ASSERT_EQ((int)(sizeof(buf) + sizeof(buf2)),
	    ioctx.read("foo", bl3, (sizeof(buf) + sizeof(buf2)), 0));
  const char *bl3_str = bl3.c_str();
  ASSERT_EQ(0, memcmp(bl3_str, buf, sizeof(buf)));
  ASSERT_EQ(0, memcmp(bl3_str + sizeof(buf), buf2, sizeof(buf2)));
}

TEST_F(LibRadosIo, TruncTest) {
  char buf[128];
  char buf2[sizeof(buf)];
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0, rados_trunc(ioctx, "foo", sizeof(buf) / 2));
  memset(buf2, 0, sizeof(buf2));
  ASSERT_EQ((int)(sizeof(buf)/2), rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
  ASSERT_EQ(0, memcmp(buf, buf2, sizeof(buf)/2));
}

TEST_F(LibRadosIoPP, TruncTestPP) {
  char buf[128];
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl, sizeof(buf)));
  ASSERT_EQ(0, ioctx.trunc("foo", sizeof(buf) / 2));
  bufferlist bl2;
  ASSERT_EQ((int)(sizeof(buf)/2), ioctx.read("foo", bl2, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(bl2.c_str(), buf, sizeof(buf)/2));
}

TEST_F(LibRadosIo, RemoveTest) {
  char buf[128];
  char buf2[sizeof(buf)];
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0, rados_remove(ioctx, "foo"));
  memset(buf2, 0, sizeof(buf2));
  ASSERT_EQ(-ENOENT, rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
}

TEST_F(LibRadosIoPP, RemoveTestPP) {
  char buf[128];
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  ASSERT_EQ(0, ioctx.remove("foo"));
  bufferlist bl2;
  ASSERT_EQ(-ENOENT, ioctx.read("foo", bl2, sizeof(buf), 0));
}

TEST_F(LibRadosIo, XattrsRoundTrip) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(-ENODATA, rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ((int)sizeof(attr1_buf),
	    rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
  ASSERT_EQ(0, memcmp(attr1_buf, buf, sizeof(attr1_buf)));
}

TEST_F(LibRadosIoPP, XattrsRoundTripPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  ASSERT_EQ(-ENODATA, ioctx.getxattr("foo", attr1, bl2));
  bufferlist bl3;
  bl3.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl3));
  bufferlist bl4;
  ASSERT_EQ((int)sizeof(attr1_buf),
      ioctx.getxattr("foo", attr1, bl4));
  ASSERT_EQ(0, memcmp(bl4.c_str(), attr1_buf, sizeof(attr1_buf)));
}

TEST_F(LibRadosIo, RmXattr) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0,
      rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ(0, rados_rmxattr(ioctx, "foo", attr1));
  ASSERT_EQ(-ENODATA, rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
}

TEST_F(LibRadosIoPP, RmXattrPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  bl2.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl2));
  ASSERT_EQ(0, ioctx.rmxattr("foo", attr1));
  bufferlist bl3;
  ASSERT_EQ(-ENODATA, ioctx.getxattr("foo", attr1, bl3));
}

TEST_F(LibRadosIo, XattrIter) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  char attr2[] = "attr2";
  char attr2_buf[256];
  for (size_t j = 0; j < sizeof(attr2_buf); ++j) {
    attr2_buf[j] = j % 0xff;
  }
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr2, attr2_buf, sizeof(attr2_buf)));
  rados_xattrs_iter_t iter;
  ASSERT_EQ(0, rados_getxattrs(ioctx, "foo", &iter));
  int num_seen = 0;
  while (true) {
    const char *name;
    const char *val;
    size_t len;
    ASSERT_EQ(0, rados_getxattrs_next(iter, &name, &val, &len));
    if (name == NULL) {
      break;
    }
    ASSERT_LT(num_seen, 2);
    if ((strcmp(name, attr1) == 0) && (memcmp(val, attr1_buf, len) == 0)) {
      num_seen++;
      continue;
    }
    else if ((strcmp(name, attr2) == 0) && (memcmp(val, attr2_buf, len) == 0)) {
      num_seen++;
      continue;
    }
    else {
      ASSERT_EQ(0, 1);
    }
  }
  rados_getxattrs_end(iter);
}

TEST_F(LibRadosIoPP, XattrListPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  char attr2[] = "attr2";
  char attr2_buf[256];
  for (size_t j = 0; j < sizeof(attr2_buf); ++j) {
    attr2_buf[j] = j % 0xff;
  }
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  bl2.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl2));
  bufferlist bl3;
  bl3.append(attr2_buf, sizeof(attr2_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr2, bl3));
  std::map<std::string, bufferlist> attrset;
  ASSERT_EQ(0, ioctx.getxattrs("foo", attrset));
  for (std::map<std::string, bufferlist>::iterator i = attrset.begin();
       i != attrset.end(); ++i) {
    if (i->first == string(attr1)) {
      ASSERT_EQ(0, memcmp(i->second.c_str(), attr1_buf, sizeof(attr1_buf)));
    }
    else if (i->first == string(attr2)) {
      ASSERT_EQ(0, memcmp(i->second.c_str(), attr2_buf, sizeof(attr2_buf)));
    }
    else {
      ASSERT_EQ(0, 1);
    }
  }
}

TEST_F(LibRadosIoEC, SimpleWrite) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  rados_ioctx_set_namespace(ioctx, "nspace");
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
}

TEST_F(LibRadosIoECPP, SimpleWritePP) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
  ioctx.set_namespace("nspace");
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
}

TEST_F(LibRadosIoECPP, ReadOpPP) {
  char buf[128];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));

  {
      bufferlist op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl, op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist op_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl, op_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl1, read_bl2, op_bl;
      int rval1 = 1000, rval2 = 1002;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl1, &rval1);
      op.read(0, sizeof(buf), &read_bl2, &rval2);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), read_bl1.length());
      ASSERT_EQ(sizeof(buf), read_bl2.length());
      ASSERT_EQ(sizeof(buf) * 2, op_bl.length());
      ASSERT_EQ(0, rval1);
      ASSERT_EQ(0, rval2);
      ASSERT_EQ(0, memcmp(read_bl1.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl2.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(op_bl.c_str() + sizeof(buf), buf, sizeof(buf)));
  }

  {
      bufferlist op_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, &op_bl));
      ASSERT_EQ(sizeof(buf), op_bl.length());
      ASSERT_EQ(0, memcmp(op_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, NULL);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), NULL, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(0, rval);
  }

  {
      bufferlist read_bl;
      int rval = 1000;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl, &rval);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl.length());
      ASSERT_EQ(0, rval);
      ASSERT_EQ(0, memcmp(read_bl.c_str(), buf, sizeof(buf)));
  }

  {
      bufferlist read_bl1, read_bl2;
      int rval1 = 1000, rval2 = 1002;
      ObjectReadOperation op;
      op.read(0, sizeof(buf), &read_bl1, &rval1);
      op.read(0, sizeof(buf), &read_bl2, &rval2);
      ASSERT_EQ(0, ioctx.operate("foo", &op, NULL));
      ASSERT_EQ(sizeof(buf), read_bl1.length());
      ASSERT_EQ(sizeof(buf), read_bl2.length());
      ASSERT_EQ(0, rval1);
      ASSERT_EQ(0, rval2);
      ASSERT_EQ(0, memcmp(read_bl1.c_str(), buf, sizeof(buf)));
      ASSERT_EQ(0, memcmp(read_bl2.c_str(), buf, sizeof(buf)));
  }
}

TEST_F(LibRadosIoEC, RoundTrip) {
  char buf[128];
  char buf2[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  memset(buf2, 0, sizeof(buf2));
  ASSERT_EQ((int)sizeof(buf2), rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
  ASSERT_EQ(0, memcmp(buf, buf2, sizeof(buf)));
}

TEST_F(LibRadosIoECPP, RoundTripPP) {
  char buf[128];
  char buf2[128];
  Rados cluster;
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl, sizeof(buf), 0));
  bufferlist cl;
  ASSERT_EQ((int)sizeof(buf2), ioctx.read("foo", cl, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(buf, cl.c_str(), sizeof(buf)));
}

TEST_F(LibRadosIoEC, OverlappingWriteRoundTrip) {
  int bsize = alignment;
  int dbsize = bsize * 2;
  char *buf = (char *)new char[dbsize];
  char *buf2 = (char *)new char[bsize];
  char *buf3 = (char *)new char[dbsize];
  memset(buf, 0xcc, dbsize);
  ASSERT_EQ(dbsize, rados_write(ioctx, "foo", buf, dbsize, 0));
  memset(buf2, 0xdd, sizeof(buf2));
  ASSERT_EQ(-EOPNOTSUPP, rados_write(ioctx, "foo", buf2, bsize, 0));
  memset(buf3, 0xdd, sizeof(buf3));
  ASSERT_EQ(dbsize, rados_read(ioctx, "foo", buf3, dbsize, 0));
  // Read the same as first write
  ASSERT_EQ(0, memcmp(buf3, buf, dbsize));

  delete[] buf;
  delete[] buf2;
  delete[] buf3;
}

TEST_F(LibRadosIoECPP, OverlappingWriteRoundTripPP) {
  int bsize = alignment;
  int dbsize = bsize * 2;
  char *buf = (char *)new char[dbsize];
  char *buf2 = (char *)new char[bsize];
  memset(buf, 0xcc, dbsize);
  bufferlist bl1;
  bl1.append(buf, dbsize);
  ASSERT_EQ(dbsize, ioctx.write("foo", bl1, dbsize, 0));
  memset(buf2, 0xdd, bsize);
  bufferlist bl2;
  bl2.append(buf2, bsize);
  ASSERT_EQ(-EOPNOTSUPP, ioctx.write("foo", bl2, bsize, 0));
  bufferlist bl3;
  ASSERT_EQ(dbsize, ioctx.read("foo", bl3, dbsize, 0));
  // Read the same as first write
  ASSERT_EQ(0, memcmp(bl3.c_str(), buf, dbsize));

  delete[] buf;
  delete[] buf2;
}

TEST_F(LibRadosIoEC, WriteFullRoundTrip) {
  char buf[128];
  char buf2[64];
  char buf3[128];
  memset(buf, 0xcc, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_write(ioctx, "foo", buf, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  ASSERT_EQ(0, rados_write_full(ioctx, "foo", buf2, sizeof(buf2)));
  memset(buf3, 0xdd, sizeof(buf3));
  ASSERT_EQ((int)sizeof(buf2), rados_read(ioctx, "foo", buf3, sizeof(buf3), 0));
  ASSERT_EQ(0, memcmp(buf2, buf2, sizeof(buf2)));
}

TEST_F(LibRadosIoECPP, WriteFullRoundTripPP) {
  char buf[128];
  char buf2[64];
  memset(buf, 0xcc, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.write("foo", bl1, sizeof(buf), 0));
  memset(buf2, 0xdd, sizeof(buf2));
  bufferlist bl2;
  bl2.append(buf2, sizeof(buf2));
  ASSERT_EQ(0, ioctx.write_full("foo", bl2));
  bufferlist bl3;
  ASSERT_EQ((int)sizeof(buf2), ioctx.read("foo", bl3, sizeof(buf), 0));
  ASSERT_EQ(0, memcmp(bl3.c_str(), buf2, sizeof(buf2)));
}

TEST_F(LibRadosIoEC, AppendRoundTrip) {
  char *buf = (char *)new char[alignment];
  char *buf2 = (char *)new char[alignment];
  char *buf3 = (char *)new char[alignment *2];
  memset(buf, 0xde, alignment);
  ASSERT_EQ((int)alignment, rados_append(ioctx, "foo", buf, alignment));
  memset(buf2, 0xad, alignment);
  ASSERT_EQ((int)alignment, rados_append(ioctx, "foo", buf2, alignment));
  memset(buf3, 0, alignment*2);
  ASSERT_EQ((int)alignment*2, rados_read(ioctx, "foo", buf3, alignment*2, 0));
  ASSERT_EQ(0, memcmp(buf3, buf, alignment));
  ASSERT_EQ(0, memcmp(buf3 + alignment, buf2, alignment));

  delete[] buf;
  delete[] buf2;
  delete[] buf3;
}

TEST_F(LibRadosIoECPP, AppendRoundTripPP) {
  char *buf = (char *)new char[alignment];
  char *buf2 = (char *)new char[alignment];
  memset(buf, 0xde, alignment);
  bufferlist bl1;
  bl1.append(buf, alignment);
  ASSERT_EQ((int)alignment, ioctx.append("foo", bl1, alignment));
  memset(buf2, 0xad, alignment);
  bufferlist bl2;
  bl2.append(buf2, alignment);
  ASSERT_EQ((int)alignment, ioctx.append("foo", bl2, alignment));
  bufferlist bl3;
  ASSERT_EQ((int)(alignment * 2),
	    ioctx.read("foo", bl3, (alignment * 2), 0));
  const char *bl3_str = bl3.c_str();
  ASSERT_EQ(0, memcmp(bl3_str, buf, alignment));
  ASSERT_EQ(0, memcmp(bl3_str + alignment, buf2, alignment));

  delete[] buf;
  delete[] buf2;
}

TEST_F(LibRadosIoEC, TruncTest) {
  char buf[128];
  char buf2[sizeof(buf)];
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(-EOPNOTSUPP, rados_trunc(ioctx, "foo", sizeof(buf) / 2));
  memset(buf2, 0, sizeof(buf2));
  // Same size
  ASSERT_EQ((int)sizeof(buf), rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
  // No change
  ASSERT_EQ(0, memcmp(buf, buf2, sizeof(buf)));
}

TEST_F(LibRadosIoECPP, TruncTestPP) {
  char buf[128];
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl;
  bl.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl, sizeof(buf)));
  ASSERT_EQ(-EOPNOTSUPP, ioctx.trunc("foo", sizeof(buf) / 2));
  bufferlist bl2;
  // Same size
  ASSERT_EQ((int)sizeof(buf), ioctx.read("foo", bl2, sizeof(buf), 0));
  // No change
  ASSERT_EQ(0, memcmp(bl2.c_str(), buf, sizeof(buf)));
}

TEST_F(LibRadosIoEC, RemoveTest) {
  char buf[128];
  char buf2[sizeof(buf)];
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0, rados_remove(ioctx, "foo"));
  memset(buf2, 0, sizeof(buf2));
  ASSERT_EQ(-ENOENT, rados_read(ioctx, "foo", buf2, sizeof(buf2), 0));
}

TEST_F(LibRadosIoECPP, RemoveTestPP) {
  char buf[128];
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  ASSERT_EQ(0, ioctx.remove("foo"));
  bufferlist bl2;
  ASSERT_EQ(-ENOENT, ioctx.read("foo", bl2, sizeof(buf), 0));
}

TEST_F(LibRadosIoEC, XattrsRoundTrip) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(-ENODATA, rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ((int)sizeof(attr1_buf),
	    rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
  ASSERT_EQ(0, memcmp(attr1_buf, buf, sizeof(attr1_buf)));
}

TEST_F(LibRadosIoECPP, XattrsRoundTripPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  ASSERT_EQ(-ENODATA, ioctx.getxattr("foo", attr1, bl2));
  bufferlist bl3;
  bl3.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl3));
  bufferlist bl4;
  ASSERT_EQ((int)sizeof(attr1_buf),
      ioctx.getxattr("foo", attr1, bl4));
  ASSERT_EQ(0, memcmp(bl4.c_str(), attr1_buf, sizeof(attr1_buf)));
}

TEST_F(LibRadosIoEC, RmXattr) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0,
      rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ(0, rados_rmxattr(ioctx, "foo", attr1));
  ASSERT_EQ(-ENODATA, rados_getxattr(ioctx, "foo", attr1, buf, sizeof(buf)));
}

TEST_F(LibRadosIoECPP, RmXattrPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  bl2.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl2));
  ASSERT_EQ(0, ioctx.rmxattr("foo", attr1));
  bufferlist bl3;
  ASSERT_EQ(-ENODATA, ioctx.getxattr("foo", attr1, bl3));
}

TEST_F(LibRadosIoEC, XattrIter) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  char attr2[] = "attr2";
  char attr2_buf[256];
  for (size_t j = 0; j < sizeof(attr2_buf); ++j) {
    attr2_buf[j] = j % 0xff;
  }
  memset(buf, 0xaa, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), rados_append(ioctx, "foo", buf, sizeof(buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr1, attr1_buf, sizeof(attr1_buf)));
  ASSERT_EQ(0, rados_setxattr(ioctx, "foo", attr2, attr2_buf, sizeof(attr2_buf)));
  rados_xattrs_iter_t iter;
  ASSERT_EQ(0, rados_getxattrs(ioctx, "foo", &iter));
  int num_seen = 0;
  while (true) {
    const char *name;
    const char *val;
    size_t len;
    ASSERT_EQ(0, rados_getxattrs_next(iter, &name, &val, &len));
    if (name == NULL) {
      break;
    }
    ASSERT_LT(num_seen, 2);
    if ((strcmp(name, attr1) == 0) && (memcmp(val, attr1_buf, len) == 0)) {
      num_seen++;
      continue;
    }
    else if ((strcmp(name, attr2) == 0) && (memcmp(val, attr2_buf, len) == 0)) {
      num_seen++;
      continue;
    }
    else {
      ASSERT_EQ(0, 1);
    }
  }
  rados_getxattrs_end(iter);
}

TEST_F(LibRadosIoECPP, XattrListPP) {
  char buf[128];
  char attr1[] = "attr1";
  char attr1_buf[] = "foo bar baz";
  char attr2[] = "attr2";
  char attr2_buf[256];
  for (size_t j = 0; j < sizeof(attr2_buf); ++j) {
    attr2_buf[j] = j % 0xff;
  }
  memset(buf, 0xaa, sizeof(buf));
  bufferlist bl1;
  bl1.append(buf, sizeof(buf));
  ASSERT_EQ((int)sizeof(buf), ioctx.append("foo", bl1, sizeof(buf)));
  bufferlist bl2;
  bl2.append(attr1_buf, sizeof(attr1_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr1, bl2));
  bufferlist bl3;
  bl3.append(attr2_buf, sizeof(attr2_buf));
  ASSERT_EQ(0, ioctx.setxattr("foo", attr2, bl3));
  std::map<std::string, bufferlist> attrset;
  ASSERT_EQ(0, ioctx.getxattrs("foo", attrset));
  for (std::map<std::string, bufferlist>::iterator i = attrset.begin();
       i != attrset.end(); ++i) {
    if (i->first == string(attr1)) {
      ASSERT_EQ(0, memcmp(i->second.c_str(), attr1_buf, sizeof(attr1_buf)));
    }
    else if (i->first == string(attr2)) {
      ASSERT_EQ(0, memcmp(i->second.c_str(), attr2_buf, sizeof(attr2_buf)));
    }
    else {
      ASSERT_EQ(0, 1);
    }
  }
}
