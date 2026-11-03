# syntax=docker/dockerfile:1
# Anthropic OSS Scanner: builds GCS develop in a public, reproducible
# AlmaLinux 8 environment, then audits it without Internet access.
FROM ghcr.io/geniusventures/almalinux-8:latest

ENV CI=1 \
    CC=/usr/bin/clang \
    CXX=/usr/bin/clang++ \
    CMAKE_GENERATOR=Ninja
ARG SCANNER_BUILD_JOBS=8

RUN dnf install -y ca-certificates curl tar gzip findutils \
 && dnf clean all
WORKDIR /src
COPY . /src
# Gitlinks are pinned by the checkout, not advanced to submodules' develop.
RUN git config --global --add url."https://github.com/".insteadOf "git@github.com:" \
 && git submodule sync --recursive \
 && git submodule update --init --recursive

# Our develop CI consumes publicly released native libraries rather than
# rebuilding all third-party projects. Verify the exact current asset digest
# so a mutable release tag cannot silently change the audited binary inputs.
# Pinned public release asset from the project-owned develop Linux x86_64 CI.
RUN mkdir -p /src/thirdparty \
 && curl -fL --retry 3 --retry-delay 4 -o /tmp/thirdparty.tar.gz \
      "https://github.com/GeniusVentures/thirdparty/releases/download/Linux-x86_64-develop-Release/Linux-x86_64-Release.tar.gz" \
 && echo "bff18072d04f7c90c0c1552d3ecd64c0e3dc2952f9ca24405d9842be64a4f5d4  /tmp/thirdparty.tar.gz" | sha256sum -c - \
 && tar -xzf /tmp/thirdparty.tar.gz -C /src/thirdparty \
 && rm -f /tmp/thirdparty.tar.gz

# Pinned public release asset from the project-owned develop Linux x86_64 CI.
RUN mkdir -p /src/zkLLVM \
 && curl -fL --retry 3 --retry-delay 4 -o /tmp/zkLLVM.tar.gz \
      "https://github.com/GeniusVentures/zkLLVM/releases/download/Linux-x86_64-develop-Release/Linux-x86_64-Release.tar.gz" \
 && echo "359ca5bbf4ac606be44fe4168419703e2e130a7a182eaceed2572bcfe65f0c11  /tmp/zkLLVM.tar.gz" | sha256sum -c - \
 && tar -xzf /tmp/zkLLVM.tar.gz -C /src/zkLLVM \
 && rm -f /tmp/zkLLVM.tar.gz

# Pinned public release asset from the project-owned develop Linux x86_64 CI.
RUN mkdir -p /src/SuperGenius \
 && curl -fL --retry 3 --retry-delay 4 -o /tmp/SuperGenius.tar.gz \
      "https://github.com/GeniusVentures/SuperGenius/releases/download/Linux-x86_64-develop-Release/Linux-x86_64-Release.tar.gz" \
 && echo "e0b3ed9f95b083c343a09c76950a9d21fcfa8d12805077b6f7a4e54474997e5a  /tmp/SuperGenius.tar.gz" | sha256sum -c - \
 && tar -xzf /tmp/SuperGenius.tar.gz -C /src/SuperGenius \
 && rm -f /tmp/SuperGenius.tar.gz

# Pinned public release asset from the project-owned develop Linux x86_64 CI.
RUN mkdir -p /src/GeniusSDK \
 && curl -fL --retry 3 --retry-delay 4 -o /tmp/GeniusSDK.tar.gz \
      "https://github.com/GeniusVentures/GeniusSDK/releases/download/Linux-x86_64-develop-Release/Linux-x86_64-Release.tar.gz" \
 && echo "fc260c2977f05b1748675f7b4145b7be25efd0c777015bf0e041de4d2cf8530e  /tmp/GeniusSDK.tar.gz" | sha256sum -c - \
 && tar -xzf /tmp/GeniusSDK.tar.gz -C /src/GeniusSDK \
 && rm -f /tmp/GeniusSDK.tar.gz

# Reproduce the Linux Release CMake configuration with prebuilt dependencies.
# Keep the source and CTest binaries in the final image for offline auditing.
RUN cmake -S /src/build/Linux -B /src/build/Linux/Release/x86_64 -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=/usr/bin/clang \
      -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
      -DTHIRDPARTY_DIR=/src/thirdparty \
      -DTHIRDPARTY_BUILD_DIR=/src/thirdparty/build/Linux/Release/x86_64 \
      -DZKLLVM_BUILD_DIR=/src/zkLLVM/build/Linux/Release/x86_64 \
      -DSUPERGENIUS_DIR=/src/SuperGenius \
      -DGENIUSSDK_DIR=/src/GeniusSDK \
      -DABI_SUBFOLDER_NAME=/x86_64 \
      -DBUILD_TESTS=ON \
 && cmake --build /src/build/Linux/Release/x86_64 --parallel "${SCANNER_BUILD_JOBS}" \
 && cmake --install /src/build/Linux/Release/x86_64 \
 && ctest --test-dir /src/build/Linux/Release/x86_64 -N | grep -q test_gcs_crypto

CMD ["/bin/bash"]
