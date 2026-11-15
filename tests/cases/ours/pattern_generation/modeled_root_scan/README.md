# Modeled root raw scans

Self-constructed IEEE1687 7.9.12 root client with an explicit linear ScanRegister
and self-capture. Raw writes alternate with ordinary read/write Applies. This
checks native state/controlling-fill continuity and vendor export of the original
and generated forms. Qualification results and actual input hashes are recorded
separately; export success alone is not a round-trip certificate.
