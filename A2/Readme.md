# DNS Resolver: A Python-based DNS Query Tool

## Overview

The **DNS Resolver** is a Python-based implementation of both **iterative** and **recursive** DNS query resolution. It showcases the hierarchical structure of the Domain Name System (DNS) by resolving domain names to IP addresses using root servers, top-level domain (TLD) servers, and authoritative name servers. The project utilizes the `dnspython` library and provides insights into how DNS works under the hood.

---

## Features

### ✦ Iterative DNS Resolution

- Begins at **root DNS servers** and follows the hierarchy to **TLD servers** and **authoritative name servers**.
- Dynamically extracts **nameserver (NS) records** to continue resolution.
- Mimics real-world iterative resolution without relying on system-configured resolvers.

### ✦ Recursive DNS Resolution

- Uses system-configured DNS resolvers (e.g., Google DNS, Cloudflare, or ISP-provided resolvers).
- Directly retrieves the final resolved IP without manual intervention.

### ✦ Real-Time Query Execution

- Measures and displays the time taken to resolve queries.
- Logs every step in the resolution process for clear debugging insights.

### ✦ Robust Error Handling

- Handles various DNS errors, including:
  - **NXDOMAIN** (Non-existent domain)
  - **Time-outs**
  - **Unreachable nameservers**
  - **No answer returned**
- Provides meaningful error messages for better troubleshooting.

### ✦ Optimized Performance

- Uses **UDP-based** DNS queries for faster resolution.
- Implements efficient NS record extraction and query handling.
- Minimizes redundant queries with smart query flow control.

---

## Code Structure

### Core Functions

#### 1. `iterative_dns_lookup(domain)`

- Starts querying from predefined **root DNS servers**.
- Extracts **NS records** and follows the DNS hierarchy.
- Continues querying until an **answer** is found or resolution **fails**.

#### 2. `recursive_dns_lookup(domain)`

- Uses the system's configured resolver for recursive resolution.
- Handles various DNS resolution errors.

#### 3. `send_dns_query(server, domain)`

- Sends a **UDP-based** DNS query to a nameserver.
- Returns the response if successful or logs an error.

#### 4. `extract_next_nameservers(response)`

- Extracts **NS records** from the authority section.
- Resolves NS names to IP addresses for the next query step.

---

## Installation & Dependencies

### Prerequisites

Ensure you have **Python 3** installed along with the required dependencies:

```bash
pip install dnspython
```

---

## Usage

Run the script using:

```bash
python3 dns_resolver.py <iterative|recursive> <domain>
```

### Example Commands

#### Iterative DNS Resolution

```bash
python3 dns_resolver.py iterative example.com
```

#### Recursive DNS Resolution

```bash
python3 dns_resolver.py recursive example.com
```

### Sample Output

```bash
[Iterative DNS Lookup] Resolving example.com
[DEBUG] Querying ROOT server (198.41.0.4) - SUCCESS
[DEBUG] Querying TLD server (192.5.6.30) - SUCCESS
[SUCCESS] example.com -> 93.184.216.34
Time taken: 0.1234 seconds
```

---

## Troubleshooting & Common Issues

### ❖ Query Timeout

- Increase the `TIMEOUT` variable in the script (default: `3` seconds).
- Ensure an active internet connection and check root server availability.

### ❖ No Answer Returned

- The queried domain may not exist (**NXDOMAIN** error).
- The authoritative nameservers may be down or misconfigured.

### ❖ Infinite Loop or No Progress

- Misconfigured responses may cause an infinite loop. Try a different domain.
- Debug extracted NS records by enabling additional logs.

---

## Performance Optimization

### ⚡ Faster Queries

- Implement **parallel queries** to multiple nameservers for quicker resolution.
- Use **caching** to reduce redundant requests.

### ⚡ Enhanced Error Handling

- Improve logging with structured JSON outputs for easier debugging.
- Implement retries with **exponential backoff** to handle temporary failures.

---

## Future Enhancements

- Support for querying **other DNS record types** (MX, TXT, CNAME, AAAA).
- Implement **DNSSEC validation** to enhance security.
- Introduce a **graphical user interface (GUI)** for user-friendly query execution.
- Optimize the resolution **algorithm** for large-scale queries.

---

## Acknowledgments

This project is inspired by real-world DNS resolution techniques and aims to provide a **deeper understanding of network protocols, DNS hierarchies, and Python-based network programming**.

---

